#include "static.h"

CHAR_* dot_Check(CHAR_ *const filename)
{
	CHAR_ *op;
	CHAR_ *last_dot;

	/*last_dot=NULL;*/
	last_dot=filename;
	for(op=&filename[strlen(filename)-1];*op!=ENCODE_('/')&&op!=filename;--op)if(*op==ENCODE_('.'))last_dot=op;
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

STATIC_CONFIG* static_Init(IO_CONFIG *io_config,CHAR_ const *arg)
{
	XML_DOC doc;
	XML_NODE *config_root;
	XML_NODE *node;
	C_STRING cache;
	STATIC_CONFIG *config;
	MIME s_mime;
	STATIC_REGEX s_regex;
	int i;
	FILE *fp;

	/*fp=fopen(arg,"r");*/
	fp=fopen(io_config->config_path,"r");
	cache=string_Create();
	if(cache==NULL)goto fail_return;
	config=(STATIC_CONFIG*)malloc(sizeof(STATIC_CONFIG));
	config->mime_list=array_Create(sizeof(MIME));
	if(config->mime_list==NULL)goto fail_return;
	if((config->regex_list=array_Create(sizeof(STATIC_REGEX)))==NULL)goto fail_return;
	config->index_page=string_Create();
	if(config->index_page==NULL)goto fail_return;
	config->error_page=string_Create();
	if(config->error_page==NULL)goto fail_return;
	xml_Open(&doc,fp,STORE_ALL);
	
	config_root=xml_Nodebyname(arg,doc.root);
	
	xml_Storedata(&config->index_page,xml_Nodebyname(ENCODE_("index_page"),config_root),&doc);
	xml_Storedata(&config->error_page,xml_Nodebyname(ENCODE_("error_page"),config_root),&doc);
	array_Remove(&config->index_page,&config->index_page[0]);
	array_Remove(&config->index_page,&config->index_page[array_Length(config->index_page)-2]);
	array_Remove(&config->error_page,&config->error_page[0]);
	array_Remove(&config->error_page,&config->error_page[array_Length(config->error_page)-2]);
	xml_Storedata(&cache,xml_Nodebyname(ENCODE_("buf_size"),config_root),&doc);
	config->buf_size=STRTOUL_(cache,NULL,0);

	node=xml_Nodebyname(ENCODE_("dirs"),config_root);
	for(i=0;i<xml_Nodechilds(node);++i)
	{
		xml_Parmbyname(&cache,ENCODE_("regex"),xml_Nodebyindex(i,node));
		array_Remove(&cache,&cache[0]);
		array_Remove(&cache,&cache[array_Length(cache)-2]);
		if(regcomp(&s_regex.preg,cache,0)<0)goto fail_return;

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

int static_Select(STATIC_CONFIG *config,HTTP_REQUEST *request,int fd)
{
	/*at this time,mod will recv all request*/
	C_STRING cache;
	int i;
	regmatch_t preg_match;
	STATIC_ID connect_id;
	struct stat file_stat;
	CHAR_ *root_dir;

	for(i=0;i<array_Length(config->regex_list);++i)
	{
		if(!regexec(&config->regex_list[i].preg,request->path,1,&preg_match,0))
		{
			root_dir=config->regex_list[i].root_dir;
			connect_id.buf=(char*)malloc(config->buf_size*sizeof(char));
			connect_id.connect_fd=fd;
			cache=string_Create();
			if(cache==NULL)goto fail_return;
			ERROR_OUT_(stderr,ENCODE_("THE PATH IS %s\n"),request->path);
			if(array_Resize(&cache,array_Length(request->path)+array_Length(root_dir))==NULL)goto fail_return;
			SNPRINTF_(cache,array_Head(cache)->array_space,ENCODE_("%s%s"),root_dir,request->path);
			array_Length(cache)=STRLEN_(cache)+1;
			if(cache[array_Length(cache)-2]==ENCODE_('/'))
			{
				ERROR_OUT_(stderr,ENCODE_("USE DEFAULT INDEX\n"));
				array_Remove(&cache,&cache[array_Length(cache)-1]);
				for(i=0;i<array_Length(config->index_page);++i)if(string_Append(&cache,&config->index_page[i])==NULL)goto fail_return;
			};
			connect_id.file_fd=open(cache,O_RDONLY);
			if(fstat(connect_id.file_fd,&file_stat)==-1)
			{
				ERROR_OUT_(stderr,ENCODE_("STAT FAILED\n"));
			};
			if(connect_id.file_fd>0)
			{
				ERROR_OUT_(stderr,ENCODE_("FILE OPEN OK!\n"));
				ERROR_OUT_(stderr,"the dot is %s\n",dot_Check(cache));
				for(i=0;i<array_Length(config->mime_list);++i)if(!strcmp(dot_Check(cache),config->mime_list[i].mime_dot))break;
				if(i>=array_Length(config->mime_list))i=array_Length(config->mime_list)-1;
				if(request->alive==TRUE_)
				{
					connect_id.alive=TRUE_;
					connect_id.buf_len=snprintf(connect_id.buf,config->buf_size,HTTP_HEADER_KEEPALIVE_,config->mime_list[i].mime_type,file_stat.st_size,10,200);
				}
				else
				{
					connect_id.alive=FALSE_;
					connect_id.buf_len=snprintf(connect_id.buf,config->buf_size,HTTP_HEADER_,config->mime_list[i].mime_type,file_stat.st_size);
				}
				ERROR_OUT_(stderr,"HEADER IS %s\n",connect_id.buf);
				if(connect_id.buf_len>=config->buf_size)goto fail_return;

				connect_id.send_buf=connect_id.buf_len;
				connect_id.send_off=0;
				connect_id.file_size=file_stat.st_size;

				string_Drop(&cache);
				hash_Append(&config->id_list,&connect_id);
				return SELECT_OUTPUT_;
			}
			else
			{
				/*404 - error*/
				/*fail here*/
				ERROR_OUT_(stderr,ENCODE_("FILE NOT FOUND\n"));
				/*goto fail_return;*/

				if(array_Resize(&cache,array_Length(config->error_page)+array_Length(root_dir))==NULL)goto fail_return;
				snprintf(cache,array_Head(cache)->array_space,ENCODE_("%s/%s"),root_dir,config->error_page);
				ERROR_OUT_(stderr,"opening 404 page:%s\n",cache);
				connect_id.file_fd=open(cache,O_RDONLY);
				if(fstat(connect_id.file_fd,&file_stat)==-1)
				{
					ERROR_OUT_(stderr,ENCODE_("STAT FAILED\n"));
				};
				if(connect_id.file_fd>0)
				{
					connect_id.alive=FALSE_;
					connect_id.buf_len=snprintf(connect_id.buf,config->buf_size,HTTP_NOT_FOUND_);
					connect_id.send_buf=connect_id.buf_len;
					connect_id.send_off=0;
					connect_id.file_size=file_stat.st_size;
					ERROR_OUT_(stderr,"SENDING:\n%s",connect_id.buf);
				}
				else
				{
					ERROR_OUT_(stderr,"404 page not found\n");
					connect_id.buf_len=snprintf(connect_id.buf,config->buf_size,HTTP_NOT_FOUND_MSG_);
					connect_id.send_buf=connect_id.buf_len;
					connect_id.send_off=0;
					connect_id.file_size=0;
					ERROR_OUT_(stderr,"SENDING:\n%s",connect_id.buf);
				}

				string_Drop(&cache);

				hash_Append(&config->id_list,&connect_id);
				return SELECT_OUTPUT_;
			};
		};
	};
	/*string_Drop(&cache);*/
	return SELECT_GOON_;
fail_return:
	
	string_Drop(&cache);
	free(connect_id.buf);
	
	return SELECT_BREAK_;
};

int static_Work(STATIC_CONFIG *config,HTTP_CONNECT *connect)
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
				/*setsockopt(id->connect_fd,SOL_SOCKET,TCP_CORK,&sock_op,sizeof(sock_op));*/
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
				while(1)
				{
#ifdef SENDFILE_ENABLE
					len=sendfile(id->connect_fd,id->file_fd,&id->send_off,config->buf_size);
#else
					if(id->send_buf<=0)
					{
						len=read(id->file_fd,id->buf,config->buf_size);
						id->buf_len=len;
						id->send_buf=len;
					}
#endif
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
#ifdef SENDFILE_ENABLE
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
#endif
#ifndef SENDFILE_ENABLE
					else if(len==0)
					{
						return WORK_CLOSE_;
					}
					else
					{
						break;
					}
#endif
#ifdef SENDFILE_ENABLE
					else
					{
					};
#endif
				};
			};
		};
	};
	return WORK_CLOSE_;
};

int static_Addport(STATIC_CONFIG *config,HTTP_CONNECT *connect,PORT_APPLY *apply)
{
	return -1;
};

int static_Closeport(STATIC_CONFIG *config,HTTP_CONNECT *connect,PORT_APPLY *apply)
{
	return -1;
};

int static_Close(STATIC_CONFIG *config,HTTP_CONNECT *connect)
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

void static_Unload(STATIC_CONFIG *config)
{
	ERROR_OUT_(stderr,ENCODE_("I KILL MYSELF!\n"));
	return;
};
