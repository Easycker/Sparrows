#include "mod_static.h"

CHAR_* dot_Check(CHAR_ const *filename)
{
	CHAR_ *op;
	CHAR_ *last_dot;

	last_dot=NULL;
	for(op=&filename[STRLEN_(filename)-1];*op!=ENCODE_('/')&&op!=filename;--op)if(*op==ENCODE_('.'))last_dot=op;
	return last_dot;
};

size_t file_Nread(C_ARRAY char **array,FILE *fp,size_t buf_len,size_t n)
{
	size_t request_len;
	size_t recv_len;

	request_len=0;
	array_Resize(array,buf_len);
	do
	{
		recv_len=fread((((char*)(*array))+request_len),sizeof(char),buf_len,fp);
		request_len+=recv_len;
		array_Head(*array)->array_length+=recv_len;
		array_Resize(array,array_Length(*array)+request_len);
	}while((recv_len>=buf_len)&&n==0?1:(request_len<n));

	return request_len;
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
	xml_Open(&doc,fp,STORE_ALL);
	config_root=xml_Nodebyname(ENCODE_("mod_static"),doc.root);
	xml_Storedata(&config->root_dir,xml_Nodebyname(ENCODE_("root_dir"),config_root),&doc);
	array_Remove(&config->root_dir,&config->root_dir[0]);
	array_Remove(&config->root_dir,&config->root_dir[array_Length(config->root_dir)-2]);
	node=xml_Nodebyname(ENCODE_("mime"),config_root);
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
	};

	string_Drop(&cache);
	xml_Close(&doc);

	PRINTF_(ENCODE_("mod_static loaded\nthe root is %S,and the mimes:\n"),config->root_dir);
	for(i=0;i<array_Length(config->mime_list);++i)PRINTF_(ENCODE_("the mime%d's dot is:%S,type is %S\n"),i,config->mime_list[i].mime_dot,config->mime_list[i].mime_type);

	return config;
};

BOOL_ mod_Main(HTTP_REQUEST *request,STATIC_CONFIG *config,int fd)
{
	FILE *fp;
	C_STRING cache;
	C_ARRAY char *cache_ansi;
	size_t file_len;
	int i;

	SET_LOCALE_("");
	cache=string_Create();
	cache_ansi=array_Create(sizeof(char));
	array_Resize(&cache,array_Length(request->path)+array_Length(config->root_dir));
	SNPRINTF_(cache,array_Head(cache)->array_space,ENCODE_("%S%S"),config->root_dir,request->path);
	PRINTF_(ENCODE_("I WILL OPEN A FILE,IT'S %S\n"),cache);
	string_Widetoansi(&cache_ansi,cache);
	fp=fopen(cache_ansi,"r");
	if(fp!=NULL)
	{
		string_Widetoansi(&cache_ansi,HTTP_OK_);
		send(fd,cache_ansi,strlen(cache_ansi),0);

		for(i=0;i<array_Length(config->mime_list);++i)if(!STRCMP_(dot_Check(request->path),config->mime_list[i].mime_dot))
		{
			PRINTF_(ENCODE_("CHECKING MIME DONE,THE TYPE IS %S\n"),config->mime_list[i].mime_type);
			array_Resize(&cache,STRLEN_(HTTP_TYPE_)+STRLEN_(config->mime_list[i].mime_type));
			SNPRINTF_(cache,array_Head(cache)->array_space,HTTP_TYPE_,config->mime_list[i].mime_type);
			string_Widetoansi(&cache_ansi,cache);
			send(fd,cache_ansi,strlen(cache_ansi),0);
			break;
		};
		PRINTF_(ENCODE_("NOW START TO READ FILE\n"));
		string_Widetoansi(&cache_ansi,NEW_LINE_);
		send(fd,cache_ansi,strlen(cache_ansi),0);
		
		file_len=file_Nread(&cache_ansi,fp,4096,0);
		send(fd,cache_ansi,file_len,0);
		fclose(fp);
		
	}
	else
	{
		string_Widetoansi(&cache_ansi,HTTP_NOT_FOUND_);
		send(fd,cache_ansi,strlen(cache_ansi),0);
	};

	string_Drop(&cache);
	array_Drop(&cache_ansi);
	

	return TRUE_;
};

void mod_Unload(STATIC_CONFIG *config)
{

};