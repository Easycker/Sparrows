#include "mod_cgi.h"

CHAR_* dot_Check(CHAR_ *const filename)
{
	CHAR_ *op;
	CHAR_ *last_dot;

	last_dot=NULL;
	for(op=&filename[STRLEN_(filename)-1];*op!=ENCODE_('/')&&op!=filename;--op)if(*op==ENCODE_('.'))last_dot=op;
	return last_dot;
};

void put_Env(CGI_ID *connect,CHAR_ const *name,CHAR_ const *value)
{
	C_STRING cache;
	C_ARRAY char *cache_ansi;
	CHAR_ s_char=ENCODE_('=');
	size_t i;
	char *env_var;

	cache=string_Create_Ex(name);
	cache_ansi=array_Create(sizeof(char));
	array_Remove(&cache,&cache[array_Length(cache)-1]);
	array_Append(&cache,&s_char);
	for(i=0;i<STRLEN_(value);++i)array_Append(&cache,&value[i]);
	s_char=ENCODE_('\0');
	array_Append(&cache,&s_char);
	string_Widetoansi(&cache_ansi,cache);
	
	env_var=(char*)malloc(sizeof(char)*array_Length(cache_ansi));
	memcpy(env_var,cache_ansi,array_Length(cache_ansi));
	array_Append(&connect->env_vars,&env_var);

	putenv(env_var);
	PRINTF_(ENCODE_("put env done,env:%s\n"),env_var);
	string_Drop(&cache);
	array_Drop(&cache_ansi);
};

CGI_CONFIG* mod_Config(FILE *fp)
{
	XML_DOC doc;
	XML_NODE *config_root;
	C_STRING cache;
	CGI_CONFIG *config;

	cache=string_Create();
	config=(CGI_CONFIG*)malloc(sizeof(CGI_CONFIG));
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

	string_Drop(&cache);
	xml_Close(&doc);

	return config;
};

CGI_ID* mod_Prepare(CGI_CONFIG *config,HTTP_REQUEST *request)
{
	int i;
	CGI_ID *connect_id;
	C_STRING cache;
	C_ARRAY char *cache_ansi;
	CHAR_ end_char;
	pid_t pid;
	int child_state;
	int f_in[2];
	int f_out[2];

	connect_id=(CGI_ID*)malloc(sizeof(CGI_ID));
	connect_id->env_vars=array_Create_Ex(sizeof(char**),&free);
	cache=string_Create();
	cache_ansi=array_Create(sizeof(char));
	SET_LOCALE_("");
	end_char=ENCODE_('\0');

	array_Resize(&cache,array_Length(request->path)+array_Length(config->root_dir));
	SNPRINTF_(cache,array_Head(cache)->array_space,ENCODE_("%S%S"),config->root_dir,request->path);
	array_Length(cache)=STRLEN_(cache)+1;
	if(cache[array_Length(cache)-2]==ENCODE_('/'))
	{
		array_Remove(&cache,&cache[array_Length(cache)-1]);
		for(i=0;i<array_Length(config->index_page);++i)array_Append(&cache,&config->index_page[i]);
	};
	string_Widetoansi(&cache_ansi,cache);
	/*ok well done,let's start to fork the cgi!*/
	/*first,set the env vars*/
	/*set the QUERY_STRING*/
	if(array_Length(request->query_string)>0)put_Env(connect_id,ENCODE_("QUERY_STRING"),request->query_string);
	/*set the COOKIES*/
	if(array_Length(request->cookies)>0)put_Env(connect_id,ENCODE_("COOKIES"),request->cookies);
	/*set the CONTENT_TYPE*/
	if(array_Length(request->content_type)>0)put_Env(connect_id,ENCODE_("CONTENT_TYPE"),request->content_type);
	/*set the CONTENT_LENGTH*/
	if(array_Length(request->content_length)>0)put_Env(connect_id,ENCODE_("CONTENT_LENGTH"),request->content_length);
	/*fork the process with args,dup2 is use for link its stdout and stdin*/
	/*ready the pipe*/
	pipe(f_in);
	pipe(f_out);
	pid=fork();
	if(pid!=0)
	{
		/*father process*/
		PRINTF_(ENCODE_("READ TO OPEN:%s\n,it's length is %u\n"),cache_ansi,array_Length(cache_ansi));
		close(f_out[1]);
		close(f_in[0]);
		if(request->type==GET)pid=wait(&child_state);
	}
	else
	{
		/*child process*/
		PRINTF_(ENCODE_("i am pid %u,i will dup a cgi\n"),getpid());
		dup2(f_out[1],fileno(stdout));
		dup2(f_in[0],fileno(stdin));
		close(f_out[0]);
		close(f_out[1]);
		close(f_in[0]);
		close(f_in[1]);
		
		execlp(cache_ansi,"",(char*)0);
		exit(0);
	};
	connect_id->pid=pid;
	connect_id->cgiin=f_in[1];
	connect_id->cgiout=f_out[0];
	string_Drop(&cache);
	string_Drop(&cache_ansi);
	return connect_id;
};

int mod_Recv(CGI_ID *connect_id,char *buf,int buf_len)
{
	write(connect_id->cgiin,buf,buf_len);
	return 0;
};

int mod_Send(CGI_ID *connect_id,char *buf,int buf_len)
{
	return read(connect_id->cgiout,buf,buf_len);
};

void mod_End(CGI_ID *connect_id)
{
	close(connect_id->cgiin);
	close(connect_id->cgiout);
	array_Drop(&connect_id->env_vars);
	kill(connect_id->pid,SIGKILL);
};

void mod_Unload(CGI_CONFIG *config)
{

};

