#include "mod_cgi.h"

CHAR_* dot_Check(CHAR_ *const filename)
{
	CHAR_ *op;
	CHAR_ *last_dot;

	last_dot=NULL;
	for(op=&filename[STRLEN_(filename)-1];*op!=ENCODE_('/')&&op!=filename;--op)if(*op==ENCODE_('.'))last_dot=op;
	return last_dot;
};

CGI_CONFIG* mod_Config(FILE *fp)
{
	XML_DOC doc;
	XML_NODE *config_root;
	XML_NODE *node;
	C_STRING cache;
	CGI_CONFIG *config;
	EXEC s_exec;
	int i;

	cache=string_Create();
	config=(STATIC_CONFIG*)malloc(sizeof(STATIC_CONFIG));
	config->exec_list=array_Create(sizeof(EXEC));
	config->root_dir=string_Create();
	config->index_page=string_Create();
	xml_Open(&doc,fp,STORE_ALL);
	config_root=xml_Nodebyname(ENCODE_("mod_cgi"),doc.root);
	xml_Storedata(&config->root_dir,xml_Nodebyname(ENCODE_("root_dir"),config_root),&doc);
	array_Remove(&config->root_dir,&config->root_dir[0]);
	array_Remove(&config->root_dir,&config->root_dir[array_Length(config->root_dir)-2]);
	xml_Storedata(&config->index_page,xml_Nodebyname(ENCODE_("index_page"),config_root),&doc);
	array_Remove(&config->index_page,&config->index_page[0]);
	array_Remove(&config->index_page,&config->index_page[array_Length(config->index_page)-2]);
	node=xml_Nodebyname(ENCODE_("exec"),config_root);
	for(i=0;i<xml_Nodechilds(node);++i)
	{
		xml_Parmbyname(&cache,ENCODE_("dot"),xml_Nodebyindex(i,node));
		array_Remove(&cache,&cache[0]);
		array_Remove(&cache,&cache[array_Length(cache)-2]);
		s_exec.exec_dot=string_Create_Ex(cache);
		xml_Parmbyname(&cache,ENCODE_("exec"),xml_Nodebyindex(i,node));
		array_Remove(&cache,&cache[0]);
		array_Remove(&cache,&cache[array_Length(cache)-2]);
		s_exec.exec_type=string_Create_Ex(cache);
		array_Append(&config->exec_list,&s_exec);
	};

	string_Drop(&cache);
	xml_Close(&doc);

	PRINTF_(ENCODE_("mod_static loaded\nthe root is %S,and the execs:\n"),config->root_dir);
	for(i=0;i<array_Length(config->exec_list);++i)PRINTF_(ENCODE_("the exec%d's dot is:%S,type is %S\n"),i,config->exec_list[i].exec_dot,config->exec_list[i].exec_type);

	return config;
};

BOOL_ mod_Main(HTTP_REQUEST *request,CGI_CONFIG *config,int fd)
{

};

void mod_Unload(CGI_CONFIG *config)
{

};
