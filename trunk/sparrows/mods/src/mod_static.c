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
	uint32_t hash=0;
	uint32_t x=0;
	int i;
	char *op;
	size_t len;

	len=sizeof(id->connect_fd);
	op=(char*)&(id->connect_fd);
	for(i=0;i<len;++i)
	{
		hash=(hash<<4)+(*op++);
		if((x=hash&0xF0000000L)!=0)
		{
			hash^= (x>>24);
			hash&=~x;
		};
	};
	return (hash&0x0000ffff);

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
	int i;
	FILE *fp;

	cache_ansi=array_Create(sizeof(char));
	string_Widetoansi(&cache_ansi,arg);
	fp=fopen(cache_ansi,"r");
	cache=string_Create();
	config=(STATIC_CONFIG*)malloc(sizeof(STATIC_CONFIG));
	config->mime_list=array_Create(sizeof(MIME));
	config->root_dir=string_Create();
	config->index_page=string_Create();
	xml_Open(&doc,fp,STORE_ALL);
	config_root=xml_Nodebyname(ENCODE_("mod_static"),doc.root);
	xml_Storedata(&config->root_dir,xml_Nodebyname(ENCODE_("root_dir"),config_root),&doc);
	array_Remove(&config->root_dir,&config->root_dir[0]);
	array_Remove(&config->root_dir,&config->root_dir[array_Length(config->root_dir)-2]);
	xml_Storedata(&config->index_page,xml_Nodebyname(ENCODE_("index_page"),config_root),&doc);
	array_Remove(&config->index_page,&config->index_page[0]);
	array_Remove(&config->index_page,&config->index_page[array_Length(config->index_page)-2]);
	xml_Storedata(&cache,xml_Nodebyname(ENCODE_("buf_size"),config_root),&doc);
	config->buf_size=STRTOUL_(cache,NULL,0);
	node=xml_Nodebyname(ENCODE_("mime"),config_root);
	PRINTF_(ENCODE_("READ BASIC CONFIG DONE\n"));
	for(i=0;i<xml_Nodechilds(node);++i)
	{
		xml_Parmbyname(&cache,ENCODE_("dot"),xml_Nodebyindex(i,node));
		array_Remove(&cache,&cache[0]);
		array_Remove(&cache,&cache[array_Length(cache)-2]);
		s_mime.mime_dot=string_Create_Ex(cache);
		xml_Parmbyname(&cache,ENCODE_("type"),xml_Nodebyindex(i,node));
		array_Remove(&cache,&cache[0]);
		array_Remove(&cache,&cache[array_Length(cache)-2]);
		s_mime.mime_type=string_Create_Ex(cache);
		array_Append(&config->mime_list,&s_mime);

		PRINTF_(ENCODE_("ADDED ONE MIME\n"));
	};

	/*Init the hash table*/
	hash_Create(&config->id_list,&static_Tinyhash,&static_Ensure,sizeof(STATIC_ID),0x0000ffff);
	
	string_Drop(&cache);
	xml_Close(&doc);

	PRINTF_(ENCODE_("mod_static loaded\nthe root is %S,and the mimes:\n"),config->root_dir);
	for(i=0;i<array_Length(config->mime_list);++i)PRINTF_(ENCODE_("the mime%d's dot is:%S,type is %S\n"),i,config->mime_list[i].mime_dot,config->mime_list[i].mime_type);
	fclose(fp);

	return config;
};

int mod_Select(STATIC_CONFIG *config,HTTP_REQUEST *request,int fd)
{
	/*at this time,mod will recv all request*/
	C_STRING cache;
	C_ARRAY char *cache_ansi;
	int i;
	CHAR_ end_char;
	STATIC_ID connect_id;

	if(request)/*here check the request*/
	{
		connect_id.buf=(char*)malloc(config->buf_size*sizeof(char));
		connect_id.connect_fd=fd;
		end_char=ENCODE_('\0');
		cache=string_Create();
		cache_ansi=array_Create(sizeof(char));
		PRINTF_(ENCODE_("THE PATH IS %s,ROOT_DIR IS\n"),request->path);
		array_Resize(&cache,array_Length(request->path)+array_Length(config->root_dir));
		SNPRINTF_(cache,array_Head(cache)->array_space,ENCODE_("%S%S"),config->root_dir,request->path);
		array_Length(cache)=STRLEN_(cache)+1;
		if(cache[array_Length(cache)-2]==ENCODE_('/'))
		{
			array_Remove(&cache,&cache[array_Length(cache)-1]);
			for(i=0;i<array_Length(config->index_page);++i)array_Append(&cache,&config->index_page[i]);
			/*array_Append(&cache,&end_char);*/
		};
		string_Widetoansi(&cache_ansi,cache);
		connect_id.file_fd=open(cache_ansi,O_RDONLY);
		if(connect_id.file_fd>0)
		{
			PRINTF_(ENCODE_("FILE OPEN OK!\n"));
			for(i=0;i<array_Length(config->mime_list);++i)if(!STRCMP_(dot_Check(cache),config->mime_list[i].mime_dot))
			{
				/*finish a mime head*/
				PRINTF_(ENCODE_("CHECKING MIME DONE,THE TYPE IS %S\n"),config->mime_list[i].mime_type);
				array_Resize(&cache,STRLEN_(HTTP_TYPE_)+STRLEN_(config->mime_list[i].mime_type));
				SNPRINTF_(cache,array_Head(cache)->array_space,HTTP_TYPE_,config->mime_list[i].mime_type);
				/*build a header*/
				memset(connect_id.buf,0,config->buf_size);
				string_Widetoansi(&cache_ansi,HTTP_OK_);
				i=array_Length(cache_ansi)-1;
				memcpy(connect_id.buf,cache_ansi,i);
				string_Widetoansi(&cache_ansi,cache);
				memcpy(&connect_id.buf[i],cache_ansi,array_Length(cache_ansi)-1);
				i+=array_Length(cache_ansi)-1;
				string_Widetoansi(&cache_ansi,NEW_LINE_);
				memcpy(&connect_id.buf[i],cache_ansi,(array_Length(cache_ansi)-1));
				i+=array_Length(cache_ansi)-1;
				connect_id.buf_len=read(connect_id.file_fd,&connect_id.buf[i],config->buf_size-i);
				connect_id.buf_len=config->buf_size;

				break;
			}
			else
			{
				/*use the default mime*/
			};
		}
		else
		{
			/*faile here*/
			PRINTF_(ENCODE_("FILE NOT FOUND\n"));
			goto fail_return;
		};

		string_Drop(&cache);
		array_Drop(&cache_ansi);

		hash_Append(&config->id_list,&connect_id);
		return SELECT_OUTPUT_;
		fail_return:
		
		string_Drop(&cache);
		array_Drop(&cache_ansi);
		free(connect_id.buf);
		
		return SELECT_GOON_;
	};
	return SELECT_GOON_;
};

int mod_Work(STATIC_CONFIG *config,HTTP_CONNECT *connect)
{
	STATIC_ID fake_id;
	STATIC_ID *id;

	fake_id.connect_fd=connect->fd;
	id=hash_Get(&config->id_list,&fake_id);
	if(id->buf_len>0)
	{
		id->buf_len-=send(id->connect_fd,&(id->buf[config->buf_size-id->buf_len]),id->buf_len,0);
		return WORK_GOON_;
	}
	else
	{
		id->buf_len=read(id->file_fd,id->buf,config->buf_size);
		/*here remove the connect*/
		if(id->buf_len<1)
		{
			close(id->connect_fd);
			close(id->file_fd);
			hash_Remove(&config->id_list,connect);
			return WORK_CLOSE_;
		}
		else if(id->buf_len<config->buf_size)
		{
			send(id->connect_fd,id->buf,id->buf_len,0);
			close(id->connect_fd);
			close(id->file_fd);
			hash_Remove(&config->id_list,connect);
			return WORK_CLOSE_;
		};
	};
	return WORK_CLOSE_;
};

int mod_Addport(STATIC_CONFIG *config,HTTP_CONNECT *connect,PORT_APPLY *apply)
{
	return WORK_CLOSE_;
};

void mod_Unload(STATIC_CONFIG *config)
{
	PRINTF_(ENCODE_("I KILL MYSELF!\n"));
	return;
};
