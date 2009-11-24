#include "rewrite.h"

REWRITE_CONFIG* rewrite_Init(IO_CONFIG *io_config,CHAR_ const *arg)
{
	XML_DOC doc;
	XML_NODE *config_root;
	XML_NODE *node;
	C_STRING cache;
	REWRITE_CONFIG *config;
	REWRITE_REGEX s_regex;
	int i;
	FILE *fp;

	fp=fopen(io_config->config_path,"r");
	if((cache=string_Create())==NULL)goto fail_return;
	if(cache==NULL)goto fail_return;
	config=(REWRITE_CONFIG*)malloc(sizeof(REWRITE_CONFIG));
	if((config->regex_list=array_Create(sizeof(REWRITE_REGEX)))==NULL)goto fail_return;
	xml_Open(&doc,fp,STORE_ALL);
	
	config_root=xml_Nodebyname(arg,doc.root);
	
	node=config_root;
	for(i=0;i<xml_Nodechilds(node);++i)
	{
		xml_Parmbyname(&cache,ENCODE_("regex"),xml_Nodebyindex(i,node));
		array_Remove(&cache,&cache[0]);
		array_Remove(&cache,&cache[array_Length(cache)-2]);
		if(regcomp(&s_regex.preg,cache,0)<0)goto fail_return;

		xml_Parmbyname(&cache,ENCODE_("replace"),xml_Nodebyindex(i,node));
		array_Remove(&cache,&cache[0]);
		array_Remove(&cache,&cache[array_Length(cache)-2]);
		s_regex.replace=string_Create_Ex(cache);

		array_Append(&config->regex_list,&s_regex);
	};

	string_Drop(&cache);
	xml_Close(&doc);

	fclose(fp);

	return config;
fail_return:
	return NULL;
};

int rewrite_Select(REWRITE_CONFIG *config,HTTP_REQUEST *request,int fd)
{
	C_STRING cache;
	C_STRING cache2;
	int i;
	int j;
	char *cr;
	regmatch_t preg_match[MAX_REGEX_];

	cache=string_Create();
	cache2=string_Create();
	for(i=0;i<array_Length(config->regex_list);++i)
	{
		/*changing path*/
		string_Set(&cache,request->path);
		if(!regexec(&config->regex_list[i].preg,cache,MAX_REGEX_,preg_match,0))
		{
			for(j=0;preg_match[j].rm_so>0;++j)
			{
				if(preg_match[j].rm_so<0)break;
				if((array_Length(config->regex_list[i].replace)-(preg_match[j].rm_eo-preg_match[j].rm_so))!=0)
				{
					memcpy(&cache[preg_match[j].rm_so],config->regex_list[i].replace,array_Length(config->regex_list[i].replace)-1);
					array_Resize(&request->path,array_Length(cache));
					ERROR_OUT_(stderr,ENCODE_("NOW THE ANSI IS %s\n"),cache);
					string_Set(&request->path,cache);
				};
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
					memcpy(&request->recv_data[preg_match[j].rm_so],config->regex_list[i].replace,array_Length(config->regex_list[i].replace)-1);
					cr+=(array_Length(config->regex_list[i].replace)-1-(preg_match[j].rm_eo-preg_match[j].rm_so));
					*cr='\r';
				};
			}
			else goto fail_return;
		}
		else goto fail_return;
		ERROR_OUT_(stderr,ENCODE_("REWRITE DONE,PATH IS %s\ndata is %s\n"),request->path,request->recv_data);
	};

	return SELECT_GOON_;
fail_return:
	ERROR_PRINT_;
	return SELECT_GOON_;
};

int rewrite_Work(REWRITE_CONFIG *config,HTTP_CONNECT *connect)
{
	return WORK_CLOSE_;
};

int rewrite_Addport(REWRITE_CONFIG *config,HTTP_CONNECT *connect,PORT_APPLY *apply)
{
	return 0;
};

int rewrite_Closeport(REWRITE_CONFIG *config,HTTP_CONNECT *connect,PORT_APPLY *apply)
{
	return 0;
};

int rewrite_Close(REWRITE_CONFIG *config,HTTP_CONNECT *connect)
{
	return 0;
};

void rewrite_Unload(REWRITE_CONFIG *config)
{
	ERROR_OUT_(stderr,ENCODE_("I KILL MYSELF!\n"));
	return;
};
