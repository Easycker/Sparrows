#include "mod_static.h"

CHAR_* dot_Check(CHAR_ *const filename)
{
	CHAR_ *op;
	CHAR_ *last_dot;

	last_dot=NULL;
	for(op=&filename[STRLEN_(filename)-1];*op!=ENCODE_('/')&&op!=filename;--op)if(*op==ENCODE_('.'))last_dot=op;
	return last_dot;
};

STATIC_CONFIG* mod_Config(FILE *fp)
{
	XML_DOC doc;
	XML_NODE *config_root;
	XML_NODE *node;
	C_STRING cache;
	STATIC_CONFIG *config;
	MIME s_mime;
	int i;

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

	string_Drop(&cache);
	xml_Close(&doc);

	PRINTF_(ENCODE_("mod_static loaded\nthe root is %S,and the mimes:\n"),config->root_dir);
	for(i=0;i<array_Length(config->mime_list);++i)PRINTF_(ENCODE_("the mime%d's dot is:%S,type is %S\n"),i,config->mime_list[i].mime_dot,config->mime_list[i].mime_type);

	return config;
};

int mod_Recv(STATIC_ID *connect_id,char *buf,int buf_len)
{
	return -1;
};

STATIC_ID* mod_Prepare(STATIC_CONFIG *config,HTTP_REQUEST *request)
{
	C_STRING cache;
	C_ARRAY char *cache_ansi;
	int i;
	CHAR_ end_char;
	STATIC_ID *connect_id;

	connect_id=(STATIC_ID*)malloc(sizeof(STATIC_ID));
	connect_id->mime=NULL;
	SET_LOCALE_("");
	end_char=ENCODE_('\0');
	cache=string_Create();
	cache_ansi=array_Create(sizeof(char));
	array_Resize(&cache,array_Length(request->path)+array_Length(config->root_dir));
	SNPRINTF_(cache,array_Head(cache)->array_space,ENCODE_("%S%S"),config->root_dir,request->path);
	array_Length(cache)=STRLEN_(cache)+1;
	if(cache[STRLEN_(cache)-1]==ENCODE_('/'))
	{
		array_Remove(&cache,&cache[array_Length(cache)-1]);
		for(i=0;i<array_Length(config->index_page);++i)array_Append(&cache,&config->index_page[i]);
		array_Append(&cache,&end_char);
	};
	string_Widetoansi(&cache_ansi,cache);
	connect_id->fp=fopen(cache_ansi,"r");
	if(connect_id->fp!=NULL)
	{
		PRINTF_(ENCODE_("FILE OPEN OK!\n"));
		for(i=0;i<array_Length(config->mime_list);++i)if(!STRCMP_(dot_Check(cache),config->mime_list[i].mime_dot))
		{
			/*finish a mime head*/
			PRINTF_(ENCODE_("CHECKING MIME DONE,THE TYPE IS %S\n"),config->mime_list[i].mime_type);
			array_Resize(&cache,STRLEN_(HTTP_TYPE_)+STRLEN_(config->mime_list[i].mime_type));
			SNPRINTF_(cache,array_Head(cache)->array_space,HTTP_TYPE_,config->mime_list[i].mime_type);
			connect_id->mime=string_Create_Ex(cache);
			break;
		};
		connect_id->state=0;
	}
	else
	{
		/*faile here*/
		PRINTF_(ENCODE_("FILE NOT FOUND\n"));
		return NULL;
	};

	string_Drop(&cache);
	array_Drop(&cache_ansi);

	return connect_id;
};

int mod_Send(STATIC_ID *connect_id,char *buf,int buf_len)
{
	C_ARRAY char *cache_ansi;
	char *op;
	int i;
	int count;

	if(connect_id->state==0)
	{
		cache_ansi=array_Create(sizeof(char));
		string_Widetoansi(&cache_ansi,connect_id->mime);
		i=array_Length(cache_ansi)-1;
		memcpy(buf,cache_ansi,i);
		string_Widetoansi(&cache_ansi,NEW_LINE_);
		memcpy(&buf[i],cache_ansi,(array_Length(cache_ansi)-1));
		i+=array_Length(cache_ansi)-1;
		connect_id->state=1;
		count=fread(&buf[i],sizeof(char),buf_len-i,connect_id->fp);
		array_Drop(&cache_ansi);
		return i+count;
	}
	else
	{
		return fread(buf,sizeof(char),buf_len,connect_id->fp);
	};

};

void mod_End(STATIC_ID *connect_id)
{
	fclose(connect_id->fp);
	if(connect_id->mime!=NULL)string_Drop(&connect_id->mime);
};

void mod_Unload(STATIC_CONFIG *config)
{
	int i;
	int j;

	PRINTF_(ENCODE_("I KILL MYSELF!\n"));
	return;
};
