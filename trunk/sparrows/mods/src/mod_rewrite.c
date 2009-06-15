#include "mod_rewrite.h"

REWRITE_CONFIG* mod_Init(CHAR_ const *arg)
{
	XML_DOC doc;
	XML_NODE *config_root;
	XML_NODE *node;
	C_STRING cache;
	C_ARRAY char* cache_ansi;
	REWRITE_CONFIG *config;
	REGEX_NODE s_regex;
	int i;
	FILE *fp;

	if((cache_ansi=array_Create(sizeof(char)))==NULL)goto fail_return;
	if(string_Widetoansi(&cache_ansi,arg)==NULL)goto fail_return;
	fp=fopen(cache_ansi,"r");
	if((cache=string_Create())==NULL)goto fail_return;
	if(cache==NULL)goto fail_return;
	config=(REWRITE_CONFIG*)malloc(sizeof(REWRITE_CONFIG));
	if((config->regex_list=array_Create(sizeof(REGEX_NODE)))==NULL)goto fail_return;
	xml_Open(&doc,fp,STORE_ALL);
	
	config_root=xml_Nodebyname(ENCODE_("mod_rewrite"),doc.root);
	
	node=config_root;
	for(i=0;i<xml_Nodechilds(node);++i)
	{
		xml_Parmbyname(&cache,ENCODE_("regex"),xml_Nodebyindex(i,node));
		array_Remove(&cache,&cache[0]);
		array_Remove(&cache,&cache[array_Length(cache)-2]);
		if(string_Widetoansi(&cache_ansi,cache)==NULL)goto fail_return;
		if(regcomp(&s_regex.preg,cache_ansi,0)<0)goto fail_return;

		xml_Parmbyname(&cache,ENCODE_("replace"),xml_Nodebyindex(i,node));
		array_Remove(&cache,&cache[0]);
		array_Remove(&cache,&cache[array_Length(cache)-2]);
		s_regex.replace=string_Create_Ex(cache);

		array_Append(&config->regex_list,&s_regex);
	};

	string_Drop(&cache);
	array_Drop(&cache_ansi);
	xml_Close(&doc);

	fclose(fp);

	return config;
fail_return:
	return NULL;
};

int mod_Select(REWRITE_CONFIG *config,HTTP_REQUEST *request,int fd)
{
	C_ARRAY char *cache_ansi;
	C_ARRAY char *cache_ansi2;
	C_STRING cache;
	int i;
	int j;
	int match;
	char *cr;
	regmatch_t preg_match[MAX_REGEX_];

	cache_ansi=array_Create(sizeof(char));
	cache_ansi2=array_Create(sizeof(char));
	cache=string_Create();
	for(i=0;i<array_Length(config->regex_list);++i)
	{
		/*changing path*/
		string_Widetoansi(&cache_ansi,request->path);
		if(!regexec(&config->regex_list[i].preg,cache_ansi,MAX_REGEX_,preg_match,0))
		{
			for(j=0;preg_match[j].rm_so>0;++j)
			{
				if(preg_match[j].rm_so<0)break;
				if((array_Length(config->regex_list[i].replace)-(preg_match[j].rm_eo-preg_match[j].rm_so))!=0)
				{
					array_Resize(&cache_ansi,array_Length(cache_ansi)+((array_Length(config->regex_list[i].replace)-1-(preg_match[j].rm_eo-preg_match[j].rm_so))>0?(array_Length(config->regex_list[i].replace)-1-(preg_match[j].rm_eo-preg_match[j].rm_so)):0));
					memmove(&cache_ansi[preg_match[j].rm_so+(array_Length(config->regex_list[i].replace)-1-(preg_match[j].rm_eo-preg_match[j].rm_so))],&cache_ansi[preg_match[j].rm_so],(&cache_ansi[array_Length(cache_ansi)]-&cache_ansi[preg_match[j].rm_so])/sizeof(char));
				};
				string_Widetoansi(&cache_ansi2,config->regex_list[i].replace);
				memcpy(&cache_ansi[preg_match[j].rm_so],cache_ansi2,array_Length(cache_ansi2)-1);
				array_Resize(&request->path,array_Length(cache_ansi));
				ERROR_OUT_(stderr,ENCODE_("NOW THE ANSI IS %s\n"),cache_ansi);
				string_Set(&request->path,string_Ansitowide(&cache,cache_ansi));
			};
		}
		else goto fail_return;
		/*the request*/
		/*use the first line*/
		for(cr=request->recv_data;*cr!='\n'&&((cr-request->recv_data)/sizeof(char))<request->recv_len;++cr)
		{
			if(*cr=='\r')
			{
				*cr='\0';
				break;
			};
		};
		if(*cr=='\0')
		{
			if(!regexec(&config->regex_list[i].preg,request->recv_data,MAX_REGEX_,preg_match,0))
			{
				for(j=0;preg_match[j].rm_so>0;++j)
				{
					if(preg_match[j].rm_so<0)break;
					memmove(&request->recv_data[preg_match[j].rm_so+(array_Length(config->regex_list[i].replace)-1-(preg_match[j].rm_eo-preg_match[j].rm_so))],&request->recv_data[preg_match[j].rm_so],(&request->recv_data[request->recv_len-1]-&request->recv_data[preg_match[j].rm_so])/sizeof(char));
					string_Widetoansi(&cache_ansi2,config->regex_list[i].replace);
					memcpy(&request->recv_data[preg_match[j].rm_so],cache_ansi2,array_Length(cache_ansi2)-1);
					cr+=(array_Length(config->regex_list[i].replace)-1-(preg_match[j].rm_eo-preg_match[j].rm_so));
					*cr='\r';
				};
			}
			else goto fail_return;
		}
		else goto fail_return;
		ERROR_OUT_(stderr,ENCODE_("REWRITE DONE,PATH IS %S\ndata is %s\n"),request->path,request->recv_data);
	};

	return SELECT_GOON_;
fail_return:
	ERROR_PRINT_;
	return SELECT_GOON_;
};

int mod_Work(REWRITE_CONFIG *config,HTTP_CONNECT *connect)
{
	return WORK_CLOSE_;
};

int mod_Addport(REWRITE_CONFIG *config,HTTP_CONNECT *connect,PORT_APPLY *apply)
{
	return 0;
};

int mod_Closeport(REWRITE_CONFIG *config,HTTP_CONNECT *connect,PORT_APPLY *apply)
{
	return 0;
};

int mod_Close(REWRITE_CONFIG *config,HTTP_CONNECT *connect)
{
	return 0;
};

void mod_Unload(REWRITE_CONFIG *config)
{
	ERROR_OUT_(stderr,ENCODE_("I KILL MYSELF!\n"));
	return;
};
