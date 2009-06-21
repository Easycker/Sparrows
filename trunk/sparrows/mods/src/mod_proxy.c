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
	struct hostent *he;
	int i;
	FILE *fp;

#ifndef WITHOUT_WIDECHAR_SUPPORT_
	if((cache_ansi=array_Create(sizeof(char)))==NULL)goto fail_return;
	if(string_Widetoansi(&cache_ansi,arg)==NULL)goto fail_return;
	fp=fopen(cache_ansi,"r");
#else
	fp=fopen(arg,"r");
#endif
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
		memset(&s_regex.addr,0,sizeof(s_regex.addr));
		s_regex.addr.sin_family=AF_INET;
		xml_Parmbyname(&cache,ENCODE_("regex"),xml_Nodebyindex(i,node));
		array_Remove(&cache,&cache[0]);
		array_Remove(&cache,&cache[array_Length(cache)-2]);
#ifndef WITHOUT_WIDECHAR_SUPPORT_
		if(string_Widetoansi(&cache_ansi,cache)==NULL)goto fail_return;
		if(regcomp(&s_regex.preg,cache_ansi,0)<0)goto fail_return;
#else
		if(regcomp(&s_regex.preg,cache,0)<0)goto fail_return;
#endif

		xml_Parmbyname(&cache,ENCODE_("addr"),xml_Nodebyindex(i,node));
		array_Remove(&cache,&cache[0]);
		array_Remove(&cache,&cache[array_Length(cache)-2]);
#ifndef WITHOUT_WIDECHAR_SUPPORT_
		if((he=gethostbyname(string_Widetoansi(&cache_ansi,cache)))==NULL)goto fail_return;
#else
		if((he=gethostbyname(cache))==NULL)goto fail_return;
#endif
		s_regex.addr.sin_addr=*((struct in_addr*)he->h_addr_list[0]);
		/*if(inet_aton(string_Widetoansi(&cache_ansi,cache),&s_regex.addr.sin_addr)<0)goto fail_return;*/
#ifndef WITHOUT_WIDECHAR_SUPPORT_
		ERROR_OUT_(stderr,ENCODE_("ADD A ADDRESS:%s\n"),cache_ansi);
#else
		ERROR_OUT_(stderr,ENCODE_("ADD A ADDRESS:%s\n"),cache);
#endif

		xml_Parmbyname(&cache,ENCODE_("port"),xml_Nodebyindex(i,node));
		array_Remove(&cache,&cache[0]);
		array_Remove(&cache,&cache[array_Length(cache)-2]);
		s_regex.addr.sin_port=htons(STRTOUL_(cache,NULL,0));
		ERROR_OUT_(stderr,ENCODE_("ADD A PORT:%d\n"),STRTOUL_(cache,NULL,0));

		array_Append(&config->regex_list,&s_regex);
	};

	/*Init the hash table*/
	hash_Create(&config->id_list,&proxy_Tinyhash,&proxy_Ensure,sizeof(PROXY_ID),(UINT_)HASH_SPACE_+1);
	dchain_Create(&config->owner_list,sizeof(PROXY_OWNER));
	
	string_Drop(&cache);
#ifndef WITHOUT_WIDECHAR_SUPPORT_
	array_Drop(&cache_ansi);
#endif
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
	PROXY_OWNER *id_owner;
	int remote_fd;
	int len;

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
			if((remote_fd=socket(AF_INET,SOCK_STREAM,0))<0)goto fail_return;
			if(fd_Setnonblocking(remote_fd)<0)goto fail_return;
			if((connect(remote_fd,(struct sockaddr*)(&config->regex_list[i].addr),sizeof(struct sockaddr)))<0)
			{
				if(errno==EINPROGRESS)
				{
					ERROR_OUT_(stderr,ENCODE_("CONNECTING PROXY\n"));
					connect_owner.in_fd=fd;
					connect_owner.remote_fd=remote_fd;
					connect_owner.in_read=FALSE_;
					connect_owner.in_write=FALSE_;
					connect_owner.remote_read=FALSE_;
					connect_owner.remote_write=FALSE_;
					/*connect_owner.last_op=LOCAL_SEND_REMOTE_;*/
					connect_owner.last_op=CONNECTING_;
					if(pipe(connect_owner.buf_fd)<0)
					{
						ERROR_OUT_(stderr,ENCODE_("FAIL WHILE CREATE PIPE\n"));
						goto fail_return;
					};
					ERROR_OUT_(stderr,ENCODE_("PIPE CREATED\n"));
					if((len=write(connect_owner.buf_fd[1],request->recv_data,request->recv_len))<0)
					{
						ERROR_OUT_(stderr,ENCODE_("FAIL WHILE WRITE PIPE\n"));
						goto fail_return;
					};
					connect_owner.len=len;
					id_owner=dchain_Append(&connect_owner,&config->owner_list);
					connect_id.owner=id_owner;
					connect_id.fd=fd;
					if(hash_Append(&config->id_list,&connect_id)==NULL)
					{
						ERROR_OUT_(stderr,ENCODE_("ERROR WHILE APPEND ID\n"));
						goto fail_return;
					};
					connect_id.fd=remote_fd;
					if(hash_Append(&config->id_list,&connect_id)==NULL)
					{
						ERROR_OUT_(stderr,ENCODE_("ERROR WHILE APPEND ID\n"));
						goto fail_return;
					};
					ERROR_OUT_(stderr,ENCODE_("BUILD UP CONNECT DONE!,remote_fd is %d,in_fd is %d,owner's remote_fd is %d,owner's in_fd is %d\n"),remote_fd,fd,connect_id.owner->remote_fd,connect_id.owner->in_fd);
					config->new_fd=remote_fd;
#ifndef WITHOUT_WIDECHAR_SUPPORT_
					array_Drop(&cache_ansi);
#endif
					ERROR_OUT_(stderr,ENCODE_("BUILD UP PROXY\n"));
					return SELECT_OUTPUT_|SELECT_INPUT_|SELECT_NEWPORT_;
				}
				else
				{
					ERROR_OUT2_(stderr,ENCODE_("ERROR WHILE ACCEPT PROXY\n"),errno);
					goto fail_return;
				};
			}
			else
			{
				ERROR_OUT_(stderr,ENCODE_("CONNECT BUILD DONE QUICKLY\n"));
				connect_owner.in_fd=fd;
				connect_owner.remote_fd=remote_fd;
				connect_owner.in_read=FALSE_;
				connect_owner.in_write=FALSE_;
				connect_owner.remote_read=FALSE_;
				connect_owner.remote_write=TRUE_;
				connect_owner.last_op=LOCAL_SEND_REMOTE_;
				if(pipe(connect_owner.buf_fd)<0)goto fail_return;
				if((len=write(connect_owner.buf_fd[1],request->recv_data,request->recv_len))<0)goto fail_return;
				connect_owner.len=len;
				id_owner=dchain_Append(&connect_owner,&config->owner_list);
				connect_id.fd=fd;
				connect_id.owner=id_owner;
				hash_Append(&config->id_list,&connect_id);
				connect_id.fd=remote_fd;
				hash_Append(&config->id_list,&connect_id);
				config->new_fd=remote_fd;
#ifndef WITHOUT_WIDECHAR_SUPPORT_
				array_Drop(&cache_ansi);
#endif
				return SELECT_OUTPUT_|SELECT_INPUT_|SELECT_NEWPORT_;
			};
		};
	};
#ifndef WITHOUT_WIDECHAR_SUPPORT_
	array_Drop(&cache_ansi);
#endif
	return SELECT_GOON_;
fail_return:
	
	ERROR_OUT2_(stderr,ENCODE_("FAIL WHILE SELECT PROXY\n"));
	ERROR_PRINT_;
#ifndef WITHOUT_WIDECHAR_SUPPORT_
	array_Drop(&cache_ansi);
#endif
	exit(1);
	
	return SELECT_BREAK_;
};

int mod_Work(PROXY_CONFIG *config,HTTP_CONNECT *connect)
{
	PROXY_ID fake_id;
	PROXY_ID *id;
	int len;
	size_t error_len;
	int error;
	BOOL_ retry;

	ERROR_OUT_(stderr,ENCODE_("MOD_PROXY STARTED\n"));
	fake_id.fd=connect->fd;
	id=hash_Get(&config->id_list,&fake_id);
	if(id!=NULL)
	{
		if(id->fd==id->owner->in_fd)
		{
			if(connect->event&WORK_INPUT_)
			{
				ERROR_OUT_(stderr,ENCODE_("SET IN_READ TO TRUE\n"));
				id->owner->in_read=TRUE_;
			};
			if(connect->event&WORK_OUTPUT_)
			{
				ERROR_OUT_(stderr,ENCODE_("SET IN_WRITE TO TRUE\n"));
				id->owner->in_write=TRUE_;
			};
		}
		else if(id->fd==id->owner->remote_fd)
		{
			if(connect->event&WORK_INPUT_)
			{
				ERROR_OUT_(stderr,ENCODE_("SET REMOTE_READ TO TRUE\n"));
				id->owner->remote_read=TRUE_;
			};
			if(connect->event&WORK_OUTPUT_)
			{
				ERROR_OUT_(stderr,ENCODE_("SET REMOTE_WRITE TO TRUE\n"));
				id->owner->remote_write=TRUE_;
				ERROR_OUT_(stderr,ENCODE_("SEEMS CONNECT IS COMPLETE\n"));
				if(id->owner->last_op==CONNECTING_)
				{
					error_len=sizeof(error);
					if(getsockopt(id->owner->remote_fd,SOL_SOCKET,SO_ERROR,&error,(socklen_t*)&error_len)<0)goto fail_return;
					if(error==0)
					{
						id->owner->last_op=LOCAL_SEND_REMOTE_;
						id->owner->remote_write=TRUE_;
						return WORK_INPUT_|WORK_OUTPUT_;
					}
					else
					{
						ERROR_OUT_(stderr,ENCODE_("FAIL WHILE CONNECT\n"));
						goto fail_return;
					};
				};
			};
		}
		else goto fail_return;

		if(id->owner->in_read==TRUE_&&id->owner->remote_write==TRUE_)
		{
			ERROR_OUT_(stderr,ENCODE_("START SEND DATA TO REMOTE\n"));
			/*local machine send message to remote machine*/
			if(id->owner->last_op==NOTHING_||id->owner->last_op==LOCAL_SEND_REMOTE_)
			{
				/*ok start work!*/
				if(id->owner->last_op==LOCAL_SEND_REMOTE_)
				{
					do
					{
						if((len=splice(id->owner->buf_fd[0],NULL,id->owner->remote_fd,NULL,config->buf_size,SPLICE_F_NONBLOCK))<0)
						{
							if(errno==EAGAIN)
							{
								if(id->owner->len>0)
								{
									id->owner->remote_write=FALSE_;
									id->owner->last_op=LOCAL_SEND_REMOTE_;
								}
								else
								{
									id->owner->last_op=NOTHING_;
								};
								break;
								/*return WORK_GOON_;*/
							}
							else
							{
								ERROR_OUT2_(stderr,ENCODE_("ERROR WHILE SEND PIPE\n"));
								goto fail_return;
							};
						};
						if(len==0)
						{
							ERROR_PRINT_;
							goto fail_return;
						};
						id->owner->len-=len;
					}while(len>0);
				}
				if(1)
				{
					while(1)
					{
						if((len=splice(id->owner->in_fd,NULL,id->owner->buf_fd[1],NULL,config->buf_size,SPLICE_F_NONBLOCK))<0)
						{
							if(errno==EAGAIN)
							{
								id->owner->in_read=FALSE_;
								id->owner->last_op=NOTHING_;
								break;
							}
							else
							{
								ERROR_PRINT_;
								goto fail_return;
							};
						};
						id->owner->len+=len;
						if(len>0)
						{
							if((len=splice(id->owner->buf_fd[0],NULL,id->owner->remote_fd,NULL,config->buf_size,SPLICE_F_NONBLOCK))<0)
							{
								if(errno==EAGAIN)
								{
									if(id->owner->len>0)
									{
										id->owner->remote_write=FALSE_;
										id->owner->last_op=LOCAL_SEND_REMOTE_;
									}
									else
									{
										id->owner->last_op=NOTHING_;
									};
									break;
								}
								else
								{
									ERROR_PRINT_;
									goto fail_return;
								};
							};
							id->owner->len-=len;
						}
						else
						{
							ERROR_OUT_(stderr,ENCODE_("SPLICE RETURN 0\n"));
							ERROR_PRINT_;
							return WORK_CLOSE_;
						};
					};
				};
			};
		};
		if(id->owner->in_write==TRUE_&&id->owner->remote_read==TRUE_)
		{
			ERROR_OUT_(stderr,ENCODE_("START SEND DATA FROM REMOTE TO LOCAL\n"));
			do
			{
				if(id->owner->last_op==NOTHING_||id->owner->last_op==REMOTE_SEND_LOCAL_)
				{
					/*ok start work!*/
					if(id->owner->last_op==REMOTE_SEND_LOCAL_)
					{
						do
						{
							retry=FALSE_;
							if((len=splice(id->owner->buf_fd[0],NULL,id->owner->in_fd,NULL,config->buf_size,SPLICE_F_NONBLOCK))<0)
							{
								if(errno==EAGAIN)
								{
									if(id->owner->len>0)
									{
										id->owner->in_write=FALSE_;
										id->owner->last_op=REMOTE_SEND_LOCAL_;
									}
									else
									{
										id->owner->last_op=NOTHING_;
									};
									break;
									/*return WORK_GOON_;*/
								}
								else
								{
									ERROR_PRINT_;
									goto fail_return;
								};
							};
							if(len==0)
							{
								ERROR_PRINT_;
								goto fail_return;
							};
							id->owner->len-=len;
						}while(len>0);
					}
					if(1)
					{
						ERROR_OUT_(stderr,ENCODE_("REMOTE LOOP START\n"));
						while(1)
						{
							if((len=splice(id->owner->remote_fd,NULL,id->owner->buf_fd[1],NULL,config->buf_size,SPLICE_F_NONBLOCK))<0)
							{
								if(errno==EAGAIN)
								{
									if(id->owner->len>0)
									{
										id->owner->remote_read=FALSE_;
										id->owner->last_op=REMOTE_SEND_LOCAL_;
									}
									else
									{
										id->owner->last_op=NOTHING_;
										return WORK_GOON_;
									};
									break;
								}
								else
								{
									goto fail_return;
								};
							};
							id->owner->len+=len;
							ERROR_OUT_(stderr,ENCODE_("RECV %d data\n"),len);
							if(len>0)
							{
								do
								{
									if((len=splice(id->owner->buf_fd[0],NULL,id->owner->in_fd,NULL,config->buf_size,SPLICE_F_NONBLOCK))<0)
									{
										if(errno==EAGAIN)
										{
											if(id->owner->len>0)
											{
												id->owner->in_write=FALSE_;
												id->owner->last_op=REMOTE_SEND_LOCAL_;
											}
											else
											{
												id->owner->last_op=NOTHING_;
											};
											break;
											/*return WORK_GOON_;*/
										}
										else
										{
											ERROR_PRINT_;
											goto fail_return;
										};
									};
									ERROR_OUT_(stderr,ENCODE_("SEND %d data\n"),len);
									if(len==0)
									{
										ERROR_OUT_(stderr,ENCODE_("SPLICE RETURN 0,CONNECT HAD BEEN CLOSE\n"));
										ERROR_PRINT_;
										return WORK_CLOSE_;
										/*goto fail_return;*/
									};
									id->owner->len-=len;
								}while(len>0);
							}
							else
							{
								if(id->owner->len>0)
								{
									retry=TRUE_;
									id->owner->last_op=REMOTE_SEND_LOCAL_;
									id->owner->remote_read=FALSE_;
									id->owner->remote_write=FALSE_;
									id->owner->len-=len;
								}
								else
								{
									ERROR_OUT_(stderr,ENCODE_("SPLICE RETURN 0,CONNECT HAD BEEN CLOSE\n"));
									ERROR_PRINT_;
									return WORK_CLOSE_;
								};
								/*
								ERROR_OUT_(stderr,ENCODE_("SPLICE RETURN 0,CONNECT HAD BEEN CLOSE\n"));
								ERROR_PRINT_;
								return WORK_CLOSE_;
								*/
							};
						};
					};
				};
			}while(retry==TRUE_);
		};
	};
	return WORK_GOON_;

fail_return:
	ERROR_PRINT_;
	perror("");
	return WORK_CLOSE_;
};

int mod_Addport(PROXY_CONFIG *config,HTTP_CONNECT *connect,PORT_APPLY *apply)
{
	apply->fd=config->new_fd;
	return WORK_INPUT_|WORK_OUTPUT_;
	/*return WORK_GOON_;*/
};

int mod_Closeport(PROXY_CONFIG *config,HTTP_CONNECT *connect,PORT_APPLY *apply)
{
	ERROR_OUT_(stderr,ENCODE_("CLOSEPORT RECV THE FD %d\n"),config->close_fd);
	apply->fd=config->close_fd;
	return 0;
};

int mod_Close(PROXY_CONFIG *config,HTTP_CONNECT *connect)
{
	PROXY_ID fake_id;
	PROXY_ID *id;
	PROXY_ID *remote_id;
	PROXY_ID *in_id;

	ERROR_OUT_(stderr,ENCODE_("CLOSEING PROXY,connect_fd is %d\n"),connect->fd);
	fake_id.fd=connect->fd;
	id=hash_Get(&config->id_list,&fake_id);
	if(id!=NULL)
	{
		ERROR_OUT_(stderr,ENCODE_("MY OWNER'S REMOTE_FD IS %d,IN_FD IS %d\n"),id->owner->remote_fd,id->owner->in_fd);
		if(id->fd==id->owner->remote_fd)config->close_fd=id->owner->in_fd;
		else if(id->fd==id->owner->in_fd)config->close_fd=id->owner->remote_fd;
		else goto fail_return;
		/*
		close(id->owner->remote_fd);
		close(id->owner->in_fd);
		*/
		ERROR_OUT_(stderr,ENCODE_("CLOSEING PIPE\n"));
		if(close(id->owner->buf_fd[0])<0||close(id->owner->buf_fd[1])<0)
		{
			ERROR_OUT_(stderr,ENCODE_("FAIL WHILE CLOSE PIPE\n"));
		};
		fake_id.fd=id->owner->remote_fd;
		if((remote_id=hash_Get(&config->id_list,&fake_id))==NULL)
		{
			ERROR_OUT_(stderr,ENCODE_("REMOTE FD RETURNS NULL\n"));
		};
		fake_id.fd=id->owner->in_fd;
		if((in_id=hash_Get(&config->id_list,&fake_id))==NULL)
		{
			ERROR_OUT_(stderr,ENCODE_("IN FD RETURNS NULL\n"));
		};
		dchain_Remove(id->owner,&config->owner_list);
		hash_Remove(&config->id_list,in_id);
		hash_Remove(&config->id_list,remote_id);
		return WORK_CLOSEPORT_;
	};
	return 0;
fail_return:
	ERROR_OUT_(stderr,ENCODE_("CONNECT ID ISN'T IN OWNER\n"));
	exit(1);
	ERROR_PRINT_;
	return 0;
};

void mod_Unload(PROXY_CONFIG *config)
{
	ERROR_OUT_(stderr,ENCODE_("I KILL MYSELF!\n"));
	return;
};
