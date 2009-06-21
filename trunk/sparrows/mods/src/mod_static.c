#include "mod_static.h"

CHAR_* dot_Check(CHAR_ *const filename)
{
	CHAR_ *op;
	CHAR_ *last_dot;

	last_dot=NULL;
	for(op=&filename[STRLEN_(filename)-1];*op!=ENCODE_('/')&&op!=filename;--op)if(*op==ENCODE_('.'))last_dot=op;
	return last_dot;
};

UINT_ static_Tinyhash(STATIC_ID *id)
{
	return id->connect_fd&HASH_SPACE_;
};

BOOL_ static_Ensure(STATIC_ID *lhs,STATIC_ID *rhs)
{
	return lhs->connect_fd==rhs->connect_fd;
};

STATIC_CONFIG* mod_Init(CHAR_ const *arg)
{
	XML_DOC doc;
	XML_NODE *config_root;
	XML_NODE *node;
	C_STRING cache;
	C_ARRAY char* cache_ansi;
	STATIC_CONFIG *config;
	MIME s_mime;
	REGEX_NODE s_regex;
	int i;
	FILE *fp;

#ifndef WITHOUT_WIDECHAR_SUPPORT_
	if((cache_ansi=array_Create(sizeof(char)))==NULL)goto fail_return;
	if(string_Widetoansi(&cache_ansi,arg)==NULL)goto fail_return;
	fp=fopen(cache_ansi,"r");
#else
	fp=fopen(arg,"r");
#endif
	cache=string_Create();
	if(cache==NULL)goto fail_return;
	config=(STATIC_CONFIG*)malloc(sizeof(STATIC_CONFIG));
	config->mime_list=array_Create(sizeof(MIME));
	if(config->mime_list==NULL)goto fail_return;
	if((config->regex_list=array_Create(sizeof(REGEX_NODE)))==NULL)goto fail_return;
	config->index_page=string_Create();
	if(config->index_page==NULL)goto fail_return;
	xml_Open(&doc,fp,STORE_ALL);
	
	config_root=xml_Nodebyname(ENCODE_("mod_static"),doc.root);
	
	xml_Storedata(&config->index_page,xml_Nodebyname(ENCODE_("index_page"),config_root),&doc);
	array_Remove(&config->index_page,&config->index_page[0]);
	array_Remove(&config->index_page,&config->index_page[array_Length(config->index_page)-2]);
	xml_Storedata(&cache,xml_Nodebyname(ENCODE_("buf_size"),config_root),&doc);
	config->buf_size=STRTOUL_(cache,NULL,0);

	node=xml_Nodebyname(ENCODE_("dirs"),config_root);
	for(i=0;i<xml_Nodechilds(node);++i)
	{
		xml_Parmbyname(&cache,ENCODE_("regex"),xml_Nodebyindex(i,node));
		array_Remove(&cache,&cache[0]);
		array_Remove(&cache,&cache[array_Length(cache)-2]);
#ifndef WITHOUT_WIDECHAR_SUPPORT_
		if(string_Widetoansi(&cache_ansi,cache)==NULL)goto fail_return;
		if(regcomp(&s_regex.preg,cache_ansi,0)<0)goto fail_return;
#else
		if(regcomp(&s_regex.preg,cache,0)<0)goto fail_return;
#endif

		xml_Parmbyname(&cache,ENCODE_("root"),xml_Nodebyindex(i,node));
		array_Remove(&cache,&cache[0]);
		array_Remove(&cache,&cache[array_Length(cache)-2]);
		s_regex.root_dir=string_Create_Ex(cache);
		if(s_regex.root_dir==NULL)goto fail_return;
		array_Append(&config->regex_list,&s_regex);
	};

	node=xml_Nodebyname(ENCODE_("mime"),config_root);
	for(i=0;i<xml_Nodechilds(node);++i)
	{
		xml_Parmbyname(&cache,ENCODE_("dot"),xml_Nodebyindex(i,node));
		array_Remove(&cache,&cache[0]);
		array_Remove(&cache,&cache[array_Length(cache)-2]);
		s_mime.mime_dot=string_Create_Ex(cache);
		if(s_mime.mime_dot==NULL)goto fail_return;
		xml_Parmbyname(&cache,ENCODE_("type"),xml_Nodebyindex(i,node));
		array_Remove(&cache,&cache[0]);
		array_Remove(&cache,&cache[array_Length(cache)-2]);
		s_mime.mime_type=string_Create_Ex(cache);
		if(s_mime.mime_type==NULL)goto fail_return;
		if(array_Append(&config->mime_list,&s_mime)==NULL)goto fail_return;
	};

	/*Init the hash table*/
	hash_Create(&config->id_list,&static_Tinyhash,&static_Ensure,sizeof(STATIC_ID),(UINT_)HASH_SPACE_+1);
	
	string_Drop(&cache);
	xml_Close(&doc);

	fclose(fp);

	return config;
fail_return:
	return NULL;
};

int mod_Select(STATIC_CONFIG *config,HTTP_REQUEST *request,int fd)
{
	/*at this time,mod will recv all request*/
	C_STRING cache;
	C_ARRAY char *cache_ansi;
	int i;
	int j;
	regmatch_t preg_match;
	STATIC_ID connect_id;
	struct stat file_stat;
	CHAR_ *root_dir;

#ifndef WITHOUT_WIDECHAR_SUPPORT_
	cache_ansi=array_Create(sizeof(char));
	if(cache_ansi==NULL)goto fail_return;
#endif
	for(i=0;i<array_Length(config->regex_list);++i)
	{
#ifndef WITHOUT_WIDECHAR_SUPPORT_
		if(!regexec(&config->regex_list[i].preg,string_Widetoansi(&cache_ansi,request->path),1,&preg_match,0))
#else
		if(!regexec(&config->regex_list[i].preg,request->path,1,&preg_match,0))
#endif
		{
			root_dir=config->regex_list[i].root_dir;
			connect_id.buf=(char*)malloc(config->buf_size*sizeof(char));
			connect_id.connect_fd=fd;
			cache=string_Create();
			if(cache==NULL)goto fail_return;
#ifndef WITHOUT_WIDECHAR_SUPPORT_
			ERROR_OUT_(stderr,ENCODE_("THE PATH IS %S\n"),request->path);
#else
			ERROR_OUT_(stderr,ENCODE_("THE PATH IS %s\n"),request->path);
#endif
			if(array_Resize(&cache,array_Length(request->path)+array_Length(root_dir))==NULL)goto fail_return;
#ifndef WITHOUT_WIDECHAR_SUPPORT_
			SNPRINTF_(cache,array_Head(cache)->array_space,ENCODE_("%S%S"),root_dir,request->path);
#else
			SNPRINTF_(cache,array_Head(cache)->array_space,ENCODE_("%s%s"),root_dir,request->path);
#endif
			array_Length(cache)=STRLEN_(cache)+1;
			if(cache[array_Length(cache)-2]==ENCODE_('/'))
			{
				ERROR_OUT_(stderr,ENCODE_("USE DEFAULT INDEX\n"));
				array_Remove(&cache,&cache[array_Length(cache)-1]);
				for(i=0;i<array_Length(config->index_page);++i)if(string_Append(&cache,&config->index_page[i])==NULL)goto fail_return;
				/*array_Append(&cache,&end_char);*/
			};
#ifndef WITHOUT_WIDECHAR_SUPPORT_
			if(string_Widetoansi(&cache_ansi,cache)==NULL)goto fail_return;
			connect_id.file_fd=open(cache_ansi,O_RDONLY);
#else
			connect_id.file_fd=open(cache,O_RDONLY);
#endif
			if(fstat(connect_id.file_fd,&file_stat)==-1)
			{
				ERROR_OUT_(stderr,ENCODE_("STAT FAILED\n"));
			};
			if(connect_id.file_fd>0)
			{
				ERROR_OUT_(stderr,ENCODE_("FILE OPEN OK!\n"));
				for(i=0;i<array_Length(config->mime_list);++i)if(!STRCMP_(dot_Check(cache),config->mime_list[i].mime_dot))
				{
					/*build a header*/
					memset(connect_id.buf,0,config->buf_size);
					/*200 OK*/
#ifndef WITHOUT_WIDECHAR_SUPPORT_
					if(string_Widetoansi(&cache_ansi,HTTP_OK_)==NULL)goto fail_return;
					j=array_Length(cache_ansi)-1;
					memcpy(connect_id.buf,cache_ansi,j);
#else
					j=STRLEN_(HTTP_OK_);
					memcpy(connect_id.buf,HTTP_OK_,j);
#endif
					/*mime type*/
					if(array_Resize(&cache,MAX_LINE_/*STRLEN_(HTTP_TYPE_)+STRLEN_(config->mime_list[i].mime_type)*/)==NULL)goto fail_return;
					SNPRINTF_(cache,MAX_LINE_,HTTP_TYPE_,config->mime_list[i].mime_type);
					array_Length(cache)=STRLEN_(cache)+1;
#ifndef WITHOUT_WIDECHAR_SUPPORT_
					if(string_Widetoansi(&cache_ansi,cache)==NULL)goto fail_return;
					memcpy(&connect_id.buf[j],cache_ansi,array_Length(cache_ansi)-1);
					j+=array_Length(cache_ansi)-1;
#else
					memcpy(&connect_id.buf[j],cache,array_Length(cache)-1);
					j+=array_Length(cache)-1;
#endif
					/*Content-Length*/
					/*array_Resize(&cache,STRLEN_(HTTP_LENGTH_)+100);*/
					SNPRINTF_(cache,array_Head(cache)->array_space,HTTP_LENGTH_,file_stat.st_size);
					array_Length(cache)=STRLEN_(cache)+1;
#ifndef WITHOUT_WIDECHAR_SUPPORT_
					if(string_Widetoansi(&cache_ansi,cache)==NULL)goto fail_return;
					memcpy(&connect_id.buf[j],cache_ansi,(array_Length(cache_ansi)-1));
					ERROR_OUT_(stderr,ENCODE_("RESPONSE CONTENT LENGTH:%s\n"),cache_ansi);
					j+=array_Length(cache_ansi)-1;
#else
					memcpy(&connect_id.buf[j],cache,(array_Length(cache))-1);
					ERROR_OUT_(stderr,ENCODE_("RESPONSE CONTENT LENGTH:%s\n"),cache);
					j+=array_Length(cache)-1;
#endif
					/*Keep-Alive*/
					connect_id.alive=FALSE_;
					if(request->alive==TRUE_)
					{
						connect_id.alive=TRUE_;
						SNPRINTF_(cache,array_Head(cache)->array_space,HTTP_KEEP_ALIVE_,10,200);
						array_Length(cache)=STRLEN_(cache)+1;
#ifndef WITHOUT_WIDECHAR_SUPPORT_
						if(string_Widetoansi(&cache_ansi,cache)==NULL)goto fail_return;
						memcpy(&connect_id.buf[j],cache_ansi,(array_Length(cache_ansi)-1));
						ERROR_OUT_(stderr,ENCODE_("KEEP ALIVE SETED\n"));
						j+=array_Length(cache_ansi)-1;
#else
						memcpy(&connect_id.buf[j],cache,(array_Length(cache)-1));
						ERROR_OUT_(stderr,ENCODE_("KEEP ALIVE SETED\n"));
						j+=array_Length(cache)-1;
#endif
					};
					/*/r/n/r/n*/
#ifndef WITHOUT_WIDECHAR_SUPPORT_
					if(string_Widetoansi(&cache_ansi,NEW_LINE_)==NULL)goto fail_return;
					memcpy(&connect_id.buf[j],cache_ansi,(array_Length(cache_ansi)-1));
					j+=array_Length(cache_ansi)-1;
#else
					memcpy(&connect_id.buf[j],NEW_LINE_,STRLEN_(NEW_LINE_));
					j+=STRLEN_(NEW_LINE_);
#endif
					
					/*
					connect_id.buf_len=read(connect_id.file_fd,&connect_id.buf[j],config->buf_size-j);
					*/
					connect_id.buf_len=j;
					connect_id.send_buf=j;
					connect_id.send_off=0;
					connect_id.file_size=file_stat.st_size;

					break;
				};
				if(connect_id.buf_len<1)
				{
					ERROR_OUT_(stderr,ENCODE_("MIME NOT FOUND\n"));
					goto fail_return;
					/*use the default mime*/
				};
			}
			else
			{
				/*faile here*/
				ERROR_OUT_(stderr,ENCODE_("FILE NOT FOUND\n"));
				/*goto fail_return;*/

#ifndef WITHOUT_WIDECHAR_SUPPORT_
				if(string_Widetoansi(&cache_ansi,HTTP_NOT_FOUND_)==NULL)goto fail_return;
				j=array_Length(cache_ansi)-1;
				memcpy(connect_id.buf,cache_ansi,j);
#else
				j=STRLEN_(HTTP_NOT_FOUND_);
				memcpy(connect_id.buf,HTTP_NOT_FOUND_,j);
#endif

				connect_id.buf_len=j;
				connect_id.send_buf=j;
				connect_id.send_off=0;
				connect_id.file_size=0;

				string_Drop(&cache);
#ifndef WITHOUT_WIDECHAR_SUPPORT_
				array_Drop(&cache_ansi);
#endif

				hash_Append(&config->id_list,&connect_id);
				return SELECT_OUTPUT_;
			};

			string_Drop(&cache);
#ifndef WITHOUT_WIDECHAR_SUPPORT_
			array_Drop(&cache_ansi);
#endif

			hash_Append(&config->id_list,&connect_id);
			return SELECT_OUTPUT_;
		};
	};
#ifndef WITHOUT_WIDECHAR_SUPPORT_
	array_Drop(&cache_ansi);
#endif
	return SELECT_GOON_;
fail_return:
	
	string_Drop(&cache);
#ifndef WITHOUT_WIDECHAR_SUPPORT_
	array_Drop(&cache_ansi);
#endif
	free(connect_id.buf);
	
	return SELECT_BREAK_;
};

int mod_Work(STATIC_CONFIG *config,HTTP_CONNECT *connect)
{
	STATIC_ID fake_id;
	STATIC_ID *id;
	int len;
	int sock_op;

	ERROR_OUT_(stderr,ENCODE_("MOD_STATIC STARTED\n"));
	fake_id.connect_fd=connect->fd;
	id=hash_Get(&config->id_list,&fake_id);
	if(id!=NULL)
	{
		while(1)
		{
			if(id->send_buf>0)
			{
				sock_op=1;
				setsockopt(id->connect_fd,SOL_SOCKET,TCP_CORK,&sock_op,sizeof(sock_op));
				while(1)
				{
					len=send(id->connect_fd,&(id->buf[id->buf_len-id->send_buf]),id->send_buf,0);
					if(len<0)
					{
						if(errno==EAGAIN)return WORK_GOON_;
						else 
						{
							return WORK_CLOSE_;
						};
					}
					else if(len>0)id->send_buf-=len;
					else
					{
						id->send_buf=0;
						break;
					};
				};
			}
			else/*here send the file*/
			{
				if(id->buf!=NULL)
				{
					sock_op=0;
					setsockopt(id->connect_fd,SOL_SOCKET,TCP_CORK,&sock_op,sizeof(sock_op));
					free(id->buf);
					id->buf=NULL;
				};
				while(1)
				{
					len=sendfile(id->connect_fd,id->file_fd,&id->send_off,config->buf_size);
					/*id->send_off+=len;*/
					if(len<0)
					{
						if(errno==EAGAIN)
						{
							return WORK_GOON_;
						}
						else
						{
							return WORK_CLOSE_;
						};
					}
					else if(len<config->buf_size)
					{
						if(id->send_off>=id->file_size)
						{
							if(id->alive==FALSE_)
							{
								return WORK_CLOSE_;
							}
							else
							{
								return WORK_KEEP_;
							};
						};
					}
					else
					{
					};
				};
			};
		};
	};
	return WORK_CLOSE_;
};

int mod_Addport(STATIC_CONFIG *config,HTTP_CONNECT *connect,PORT_APPLY *apply)
{
	return -1;
};

int mod_Closeport(STATIC_CONFIG *config,HTTP_CONNECT *connect,PORT_APPLY *apply)
{
	return -1;
};

int mod_Close(STATIC_CONFIG *config,HTTP_CONNECT *connect)
{
	STATIC_ID fake_id;
	STATIC_ID *id;

	fake_id.connect_fd=connect->fd;
	id=hash_Get(&config->id_list,&fake_id);
	close(id->file_fd);
	if(id->buf!=NULL)
	{
		free(id->buf);
		id->buf=NULL;
	};
	hash_Remove(&config->id_list,id);
	ERROR_OUT_(stderr,ENCODE_("close\n"));
	return 0;
};

void mod_Unload(STATIC_CONFIG *config)
{
	ERROR_OUT_(stderr,ENCODE_("I KILL MYSELF!\n"));
	return;
};
