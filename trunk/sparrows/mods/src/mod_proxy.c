#include "mod_proxy.h"

UINT_ proxy_Tinyhash(PROXY_ID *id)
{
	return id->fd&HASH_SPACE_;
};

BOOL_ proxy_Ensure(PROXY_ID *lhs,PROXY_ID *rhs)
{
	return lhs->fd==rhs->fd;
};

int fd_Setnonblocking(int fd)
{
	int op;

	op=fcntl(fd,F_GETFL,0);
	if(op==-1)return op;
	fcntl(fd,F_SETFL,op|O_NONBLOCK);

	return op;
};

PROXY_CONFIG* mod_Init(CHAR_ const *arg)
{
	XML_DOC doc;
	XML_NODE *config_root;
	XML_NODE *node;
	C_STRING cache;
	C_ARRAY char* cache_ansi;
	PROXY_CONFIG *config;
	REGEX_NODE s_regex;
	int i;
	FILE *fp;

	if((cache_ansi=array_Create(sizeof(char)))==NULL)goto fail_return;
	if(string_Widetoansi(&cache_ansi,arg)==NULL)goto fail_return;
	fp=fopen(cache_ansi,"r");
	if((cache=string_Create())==NULL)goto fail_return;
	if(cache==NULL)goto fail_return;
	config=(PROXY_CONFIG*)malloc(sizeof(PROXY_CONFIG));
	if((config->regex_list=array_Create(sizeof(REGEX_NODE)))==NULL)goto fail_return;
	xml_Open(&doc,fp,STORE_ALL);
	
	config_root=xml_Nodebyname(ENCODE_("mod_proxy"),doc.root);
	
	xml_Storedata(&cache,xml_Nodebyname(ENCODE_("buf_size"),config_root),&doc);
	config->buf_size=STRTOUL_(cache,NULL,0);

	node=xml_Nodebyname(ENCODE_("proxy"),config_root);
	for(i=0;i<xml_Nodechilds(node);++i)
	{
		xml_Parmbyname(&cache,ENCODE_("regex"),xml_Nodebyindex(i,node));
		array_Remove(&cache,&cache[0]);
		array_Remove(&cache,&cache[array_Length(cache)-2]);
		if(string_Widetoansi(&cache_ansi,cache)==NULL)goto fail_return;
		if(regcomp(&s_regex.preg,cache_ansi,0)<0)goto fail_return;

		xml_Parmbyname(&cache,ENCODE_("addr"),xml_Nodebyindex(i,node));
		array_Remove(&cache,&cache[0]);
		array_Remove(&cache,&cache[array_Length(cache)-2]);
		if(inet_aton(string_Widetoansi(&cache_ansi,cache),&s_regex.addr.sin_addr)<0)goto fail_return;

		xml_Parmbyname(&cache,ENCODE_("port"),xml_Nodebyindex(i,node));
		array_Remove(&cache,&cache[0]);
		array_Remove(&cache,&cache[array_Length(cache)-2]);
		s_regex.addr.sin_port=htons(STRTOUL_(cache,NULL,0));

		array_Append(&config->regex_list,&s_regex);
	};

	/*Init the hash table*/
	hash_Create(&config->id_list,&proxy_Tinyhash,&proxy_Ensure,sizeof(PROXY_ID),(UINT_)HASH_SPACE_+1);
	
	string_Drop(&cache);
	array_Drop(&cache_ansi);
	xml_Close(&doc);

	fclose(fp);

	return config;
fail_return:
	return NULL;
};

int mod_Select(PROXY_CONFIG *config,HTTP_REQUEST *request,int fd)
{
	C_ARRAY char *cache_ansi;
	int i;
	regmatch_t preg_match;
	PROXY_ID connect_id;
	PROXY_OWNER connect_owner;
	int remote_fd;

	cache_ansi=array_Create(sizeof(char));
	if(cache_ansi==NULL)goto fail_return;
	for(i=0;i<array_Length(config->regex_list);++i)
	{
		if(!regexec(&config->regex_list[i].preg,string_Widetoansi(&cache_ansi,request->path),1,&preg_match,0))
		{
			if((remote_fd=socket(AF_INET,SOCK_STREAM,0))<0)goto fail_return;
			if(fd_Setnonblocking(remote_fd)<0)goto fail_return;
			if(connect(remote_fd,(struct sockaddr*)&config->regex_list[i].addr,sizeof(config->regex_list[i].addr))<0)
			{
				if(errno==EINPROGRESS)
				{
					connect_owner.in_fd=fd;
					connect_owner.remote_fd=remote_fd;
					connect_owner.in_ready=FALSE_;
					connect_owner.remote_ready=FALSE_;
					connect_owner.state=STATE_CONNECT_;
					chain_Append(&connect_owner,&config->owner_list);
					connect_id.fd=fd;
					connect_id.owner=(PROXY_OWNER*)chain_Tail(&config->owner_list);
					hash_Append(&config->id_list,&connect_id);
					connect_id.fd=remote_fd;
					hash_Append(&config->id_list,&connect_id);
					config->new_fd=remote_fd;
					array_Drop(&cache_ansi);
					return SELECT_INPUT_|SELECT_NEWPORT_;
				}
				else goto fail_return;
			}
			else
			{
				connect_owner.in_fd=fd;
				connect_owner.remote_fd=remote_fd;
				connect_owner.in_ready=FALSE_;
				connect_owner.remote_ready=TRUE_;
				connect_owner.state=STATE_SEND_;
				chain_Append(&connect_owner,&config->owner_list);
				connect_id.fd=fd;
				connect_id.owner=(PROXY_OWNER*)chain_Tail(&config->owner_list);
				hash_Append(&config->id_list,&connect_id);
				connect_id.fd=remote_fd;
				hash_Append(&config->id_list,&connect_id);
				config->new_fd=remote_fd;
				array_Drop(&cache_ansi);
				return SELECT_INPUT_|SELECT_NEWPORT_;
			};
		};
	};
	array_Drop(&cache_ansi);
	return SELECT_GOON_;
fail_return:
	
	array_Drop(&cache_ansi);
	
	return SELECT_BREAK_;
};

int mod_Work(PROXY_CONFIG *config,HTTP_CONNECT *connect)
{
	PROXY_ID fake_id;
	PROXY_ID *id;
	int len;
	int sock_op;

	ERROR_OUT_(stderr,ENCODE_("MOD_PROXY STARTED\n"));
	if(connect_fd.fd==connect_fd.owner->in_fd)connect_fd.owner->in_ready=TRUE_;
	else if(connect_fd.fd==connect_fd.owner->remote_fd)
	{
		if(connect_fd.owner->state==STATE_CONNECT_)
		{
			connect_fd.owner->state=STATE_SEND_;
			return WORK_OUTPUT_;
		};
		connect_fd.owner->remote_ready=TRUE_;
	};
	else goto fail_return;

	if(connect_fd.owner->in_ready==TRUE_&&connect_fd.owner->remote_ready==TRUE_)
	{
		switch(connect_fd.owner->state)
		{
			case:STATE_CONNECT_
			     {
				     /*impossible*/
				     break;
			     };
			case:STATE_SEND_
			     {
				     break;
			     };
			case:STATE_RECV_
			     {
				     break;
			     };
		};
	};

fail_return:
	return WORK_CLOSE_;
};

int mod_Addport(PROXY_CONFIG *config,HTTP_CONNECT *connect,PORT_APPLY *apply)
{
	apply->fd=config->new_fd;
	return WORK_INPUT_;
};

void mod_Unload(PROXY_CONFIG *config)
{
	ERROR_OUT_(stderr,ENCODE_("I KILL MYSELF!\n"));
	return;
};
