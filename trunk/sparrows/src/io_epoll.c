#include "io_epoll.h"

IO_CONFIG* io_Init(IO_CONFIG *config,FILE *fp)
{
	XML_DOC doc;
	XML_NODE *doc_root;
	XML_NODE *config_root;
	XML_NODE *node;
	C_STRING cache;
	C_ARRAY char *cache_ansi;
	HOST_TYPE host;
	UINT_ i;
	UINT_ j;
	UINT_ k;
	UINT_ l;
	MOD_T mod;
	void *share_mod;
	FILE *fp_;

	cache=string_Create();
	cache_ansi=array_Create(sizeof(char));
	if(cache==NULL)
	{
		ERROR_OUT_(stderr,"MEMORY LEAK\n");
		exit(-1);
	};
	if(xml_Open(&doc,fp,STORE_ALL)==NULL)goto fail_return;
	doc_root=xml_Nodebyname(ENCODE_("config"),doc.root);
	xml_Storedata(&cache,xml_Nodebyname(ENCODE_("http_interface"),doc_root),&doc);
	if(!STRCMP_(cache,ENCODE_("0")))config->addr.sin_addr.s_addr=htonl(INADDR_ANY);
	else if(inet_aton(string_Widetoansi(&cache_ansi,cache),&config->addr.sin_addr)<0)goto fail_return;

	config->addr.sin_family=AF_INET;
	xml_Storedata(&cache,xml_Nodebyname(ENCODE_("http_port"),doc_root),&doc);
	config->addr.sin_port=htons(STRTOUL_(cache,NULL,0));
	xml_Storedata(&cache,xml_Nodebyname(ENCODE_("io_buf"),doc_root),&doc);
	config->io_buf=STRTOUL_(cache,NULL,0);
	xml_Storedata(&cache,xml_Nodebyname(ENCODE_("pool_length"),doc_root),&doc);
	config->pool_length=STRTOUL_(cache,NULL,0);
	xml_Storedata(&cache,xml_Nodebyname(ENCODE_("max_header"),doc_root),&doc);
	config->max_head=STRTOUL_(cache,NULL,0);
	xml_Storedata(&cache,xml_Nodebyname(ENCODE_("pool_timeout"),doc_root),&doc);
	config->pool_timeout=STRTOUL_(cache,NULL,0);
	xml_Storedata(&cache,xml_Nodebyname(ENCODE_("keep_alive"),doc_root),&doc);
	config->keep_alive=STRTOUL_(cache,NULL,0);
	xml_Storedata(&cache,xml_Nodebyname(ENCODE_("timeout"),doc_root),&doc);
	config->timeout=STRTOUL_(cache,NULL,0);

	/*now read the config of virtual host*/
	config->host_list=array_Create(sizeof(HOST_TYPE));
	doc_root=xml_Nodebyname(ENCODE_("hosts"),doc_root);
	ERROR_OUT_(stderr,ENCODE_("CHILDS:%d\n"),xml_Nodechilds(doc_root));
	for(l=0;l<xml_Nodechilds(doc_root);++l)
	{
		xml_Storedata(&cache,xml_Nodebyname(ENCODE_("regex"),xml_Nodebyindex(l,doc_root)),&doc);
		array_Remove(&cache,&cache[0]);
		array_Remove(&cache,&cache[STRLEN_(cache)-1]);
		string_Widetoansi(&cache_ansi,cache);
		if(regcomp(&host.preg,cache_ansi,0)<0)goto fail_return;

		/*here is mod loader*/

		config_root=xml_Nodebyindex(l,doc_root);
		host.mod_config.mod_table=array_Create(sizeof(MOD_T));
		if(host.mod_config.mod_table==NULL)goto fail_return;
		/*read the mods*/
		for(i=0;i<xml_Nodechilds(xml_Nodebyname(ENCODE_("mods"),config_root));++i)
		{
			/*clean up the mod_t*/
			ERROR_OUT_(stderr,ENCODE_("XML NODE CHILDS:%d\n"),xml_Nodechilds(xml_Nodebyname(ENCODE_("mods"),config_root)));
			xml_Parmbyname(&cache,ENCODE_("path"),xml_Nodebyindex(i,xml_Nodebyname(ENCODE_("mods"),config_root)));
			array_Remove(&cache,&cache[0]);
			array_Remove(&cache,&cache[STRLEN_(cache)-1]);

			if(string_Widetoansi(&cache_ansi,cache)==NULL)goto fail_return;

			share_mod=dlopen(cache_ansi,RTLD_LAZY);
			mod.mod_lib=share_mod;
			
			mod.mod_Init=dlsym(share_mod,"mod_Init");
			mod.mod_Select=dlsym(share_mod,"mod_Select");
			mod.mod_Work=dlsym(share_mod,"mod_Work");
			mod.mod_Addport=dlsym(share_mod,"mod_Addport");
			mod.mod_Closeport=dlsym(share_mod,"mod_Closeport");
			mod.mod_Close=dlsym(share_mod,"mod_Close");
			mod.mod_Unload=dlsym(share_mod,"mod_Unload");

			ERROR_OUT_(stderr,ENCODE_("DLERROR:%s\n"),dlerror());

			xml_Parmbyname(&cache,ENCODE_("arg"),xml_Nodebyindex(i,xml_Nodebyname(ENCODE_("mods"),config_root)));
			array_Remove(&cache,&cache[0]);
			array_Remove(&cache,&cache[STRLEN_(cache)-1]);

			if(mod.mod_Init!=NULL)mod.share=(void*)mod.mod_Init(cache);
			else goto fail_return;
			if(mod.share==NULL)goto fail_return;

			array_Append(&host.mod_config.mod_table,&mod);
		};

		/*a mod had been read,append it*/
		array_Append(&config->host_list,&host);
		ERROR_OUT_(stderr,ENCODE_("ADD ONE HOST\n"));
	};
	string_Drop(&cache);
	/*xml_Close(&doc);*/
	return config;
fail_return:
	ERROR_PRINT_;
	return NULL;
};

UINT_ connect_Tinyhash(HTTP_CONNECT **connect)
{
	return (*connect)->fd&HASH_SPACE_;
};

BOOL_ connect_Ensure(HTTP_CONNECT **lhs,HTTP_CONNECT **rhs)
{
	return (*lhs)->fd==(*rhs)->fd;
};

int fd_Setnonblocking(int fd)
{
	int op;

	op=fcntl(fd,F_GETFL,0);
	if(op==-1)return op;
	fcntl(fd,F_SETFL,op|O_NONBLOCK);

	return op;
};

HTTP_CONNECT* event_Add(C_HASH *table,C_DCHAIN *chain,int epoll_fd,int fd,uint32_t event,MOD_T *mod)
{
	HTTP_CONNECT connect;
	struct epoll_event io_event;
	HTTP_CONNECT *inserted_con;

	memset(&io_event,0,sizeof(struct epoll_event));
	io_event.data.fd=fd;
	io_event.events=event;
	if(fd_Setnonblocking(fd)==-1)goto fail_return;
	
	if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,fd,&io_event)!=-1)
	{
		connect.fd=fd;
		connect.mod=mod;
		if(time(&connect.last_op)<0)goto fail_return;
		connect.op_done=FALSE_;
		if(mod!=NULL)
		{
			if(chain->dchain_head==NULL)inserted_con=dchain_Append(&connect,chain);
			else inserted_con=dchain_Insert(&connect,chain->dchain_head,chain);
			hash_Append(table,&inserted_con);
		}
		else
		{
			inserted_con=malloc(sizeof(HTTP_CONNECT));
			if(inserted_con==NULL)goto fail_return;
			memcpy(inserted_con,&connect,sizeof(HTTP_CONNECT));
			hash_Append(table,&inserted_con);
		};
		ERROR_OUT_(stderr,ENCODE_("ADD A FD,IT'S %d\n"),connect.fd);
	}
	else
	{
		ERROR_OUT2_(stderr,ENCODE_("ERROR ON ADD EVENT\n"),errno);
		goto fail_return;
	};
	return inserted_con;
fail_return:
	ERROR_PRINT_;
	return NULL;
};

void event_Mod(int epoll_fd,int fd,uint32_t event)
{
	struct epoll_event io_event;

	memset(&io_event,0,sizeof(struct epoll_event));
	io_event.data.fd=fd;
	io_event.events=event;
	if(fd_Setnonblocking(fd)==-1)goto fail_return;

	ERROR_OUT_(stderr,ENCODE_("GOING TO MOD THE ID:%d\n"));
	if(epoll_ctl(epoll_fd,EPOLL_CTL_MOD,fd,&io_event)==-1)
	{
		goto fail_return;
	};
fail_return:
	/*ERROR_PRINT_;*/
	return;
};

void event_Active(C_HASH *table,C_DCHAIN *connect_chain,HTTP_CONNECT *connect,HTTP_CONNECT **timeout,int epoll_fd)
{
	HTTP_CONNECT *next;

	if(connect_chain->dchain_length>1)
	{
		ERROR_OUT_(stderr,ENCODE_("ACTIVE START\n"));
		next=(HTTP_CONNECT*)dchain_Next(connect);
		if(connect_chain->dchain_head!=connect)
		{
			*timeout=(next==NULL?(HTTP_CONNECT*)dchain_Prev(connect):next);
			dchain_Next(dchain_Prev(connect))=dchain_Next(connect);
			if(dchain_Next(connect)!=NULL)dchain_Prev(dchain_Next(connect))=dchain_Prev(connect);
			else connect_chain->dchain_tail=dchain_Prev(connect);
			dchain_Next(connect)=connect_chain->dchain_head;
			dchain_Prev(connect_chain->dchain_head)=connect;
			dchain_Prev(connect)=NULL;
			connect_chain->dchain_head=connect;
		};
	}
	else if(connect_chain->dchain_length=1)*timeout=connect;
	time(&connect->last_op);
};

C_HASH* event_Delete(C_HASH *table,C_DCHAIN *chain,HTTP_CONNECT *connect,int epoll_fd)
{
	struct epoll_event io_event;

	ERROR_OUT_(stderr,ENCODE_("GOING TO DEL THE %d\n"),connect->fd);
	
	memset(&io_event,0,sizeof(struct epoll_event));
	io_event.data.fd=connect->fd;
	if(epoll_ctl(epoll_fd,EPOLL_CTL_DEL,connect->fd,&io_event)==-1)
	{
		ERROR_OUT_(stderr,ENCODE_("ERROR ON EVENT DELETE\n"));
		/*ERROR_PRINT_;*/
	};
	hash_Remove(table,&connect);
	dchain_Remove(connect,chain);
	
	return table;
};

int epoll_Loop(C_HASH *connect_list,C_DCHAIN *connect_chain,int epoll_fd,IO_CONFIG *config,int listenfd)
{
	struct epoll_event *events;
	int wait_fd;
	uint32_t epoll_event;
	HTTP_CONNECT fake_connect;
	HTTP_CONNECT *current_connect;
	HTTP_CONNECT *timeout_connect;
	HTTP_CONNECT *timeout_prev;
	HTTP_CONNECT *close_connect;
	int client_fd;
	struct sockaddr_in client;
	struct epoll_event io_event;
	int len;
	int i;
	int state;
	PORT_APPLY addport;
	BOOL_ retry;
	BOOL_ close_retry;

	C_STRING recv_data;
	HTTP_REQUEST request;

	MOD_T head_mod;
	time_t now_time;

	sigset_t sigs;

	/*init the head_mod*/
	head_mod.share=head_Init((config->max_head)/2,config->max_head,config->host_list);
	if(head_mod.share==NULL)goto fail_return;
	head_mod.mod_Work=&head_Work;
	head_mod.mod_Close=&head_Close;

	/*init the sigs*/
	if(sigemptyset(&sigs)<0)goto fail_return;
	if(sigaddset(&sigs,SIGINT)<0)goto fail_return;

	len=sizeof(client);
	events=(struct epoll_event*)malloc(sizeof(struct epoll_event)*(config->pool_length));
	timeout_connect=NULL;
	while(1)
	{
		wait_fd=epoll_wait(epoll_fd,events,config->pool_length,config->pool_timeout);
		if(wait_fd>0)
		{
			for(i=0;i<wait_fd;++i)
			{
				ERROR_OUT_(stderr,ENCODE_("TOTAL %d events exist,DOING the %d\n"),wait_fd,i);
				fake_connect.fd=events[i].data.fd;
				ERROR_OUT_(stderr,ENCODE_("READY TO GET,FD IS %d\n"),fake_connect.fd);
				current_connect=&fake_connect;
				current_connect=(HTTP_CONNECT*)hash_Get(connect_list,&current_connect);
				if(current_connect!=NULL)
				{
					/*set the event*/
					state=0;
					current_connect=*(HTTP_CONNECT**)current_connect;
					current_connect->event=0;
					if(events[i].events|EPOLLIN)current_connect->event|=WORK_INPUT_;
					if(events[i].events|EPOLLOUT)current_connect->event|=WORK_OUTPUT_;
					ERROR_OUT_(stderr,ENCODE_("A EXIST EVENT,IT'S ID IS:%d\n"),current_connect->fd);
					if(current_connect->fd==listenfd)
					{
						ERROR_OUT_(stderr,ENCODE_("A NEW CONNECT GOING TO BE CREATE\n"));
						client_fd=accept(current_connect->fd,(struct sockaddr*)&client,&len);
						if(client_fd<0)
						{
							ERROR_OUT2_(stderr,ENCODE_("ERROR ON ACCEPT\n"));
							if(errno==EMFILE)goto fail_return;
							if(errno==ENFILE)goto fail_return;
							break;
							/*continue;*/
						};
						event_Add(connect_list,connect_chain,epoll_fd,client_fd,EPOLLIN|EPOLLET,&head_mod);
						ERROR_OUT_(stderr,ENCODE_("NEW CONNECT FINISH\n"));
					}
					else
					{
						ERROR_OUT_(stderr,ENCODE_("EVERYTING LOOKS OK,LET'S WORK,FD IS %d\n"),current_connect->fd);
						if(current_connect->mod==&head_mod)ERROR_OUT_(stderr,ENCODE_("IS HEAD MOD\n"));
						state=0;
						state=current_connect->mod->mod_Work(current_connect->mod->share,current_connect);
						do
						{
							retry=FALSE_;
							if(state&WORK_CLOSE_)
							{
								/*delete the epoll event but not close the port*/
								timeout_connect=dchain_Prev(current_connect);
								ERROR_OUT_(stderr,ENCODE_("CONNECT IS DOWN\n"));
								if(current_connect->mod->mod_Close(current_connect->mod->share,current_connect)&WORK_CLOSEPORT_)
								{
									do
									{
										close_retry=FALSE_;
										if(current_connect->mod->mod_Closeport(current_connect->mod->share,current_connect,&addport)>0)close_retry=TRUE_;
										ERROR_OUT_(stderr,ENCODE_("CLOSE NEW_FD IS %d\n"),addport.fd);
										if(addport.fd<0)break;
										fake_connect.fd=addport.fd;
										close_connect=&fake_connect;
										close_connect=(HTTP_CONNECT*)hash_Get(connect_list,&close_connect);
										if(close_connect!=NULL)
										{
											close_connect=*(HTTP_CONNECT**)close_connect;
											close(close_connect->fd);
											event_Delete(connect_list,connect_chain,close_connect,epoll_fd);
										}
										else goto fail_return;
									}while(close_retry==TRUE_);
								};
								close(current_connect->fd);
								/*timeout_connect=NULL;*/
								event_Delete(connect_list,connect_chain,current_connect,epoll_fd);
							}
							else if(state&WORK_KEEP_)
							{
								ERROR_OUT_(stderr,ENCODE_("KEEP ALIVE\n"));
								event_Active(connect_list,connect_chain,current_connect,&timeout_connect,epoll_fd);
								current_connect->op_done=TRUE_;
								current_connect->mod->mod_Close(current_connect->mod->share,current_connect);
								current_connect->mod=&head_mod;
								event_Mod(epoll_fd,current_connect->fd,EPOLLIN|EPOLLET);
							}
							else if(!(state&WORK_GOON_))
							{
								epoll_event=0;
								epoll_event|=EPOLLET;
								if(state&WORK_INPUT_)epoll_event|=EPOLLIN;
								if(state&WORK_OUTPUT_)epoll_event|=EPOLLOUT;
								event_Mod(epoll_fd,current_connect->fd,epoll_event);
								event_Active(connect_list,connect_chain,current_connect,&timeout_connect,epoll_fd);
							}
							else
							{
								/*here goon*/
								event_Active(connect_list,connect_chain,current_connect,&timeout_connect,epoll_fd);
							};
							if(state&WORK_NEWPORT_)
							{
								/*a new port here,understand?*/
								ERROR_OUT_(stderr,ENCODE_("NEW PORT HERE\n"));
								state=0;
								state=current_connect->mod->mod_Addport(current_connect->mod->share,current_connect,&addport);
								current_connect=event_Add(connect_list,connect_chain,epoll_fd,addport.fd,EPOLLIN|EPOLLET,current_connect->mod);
								retry=TRUE_;
							};
						}while(retry==TRUE_);
					};
				}
				else
				{
					ERROR_OUT_(stderr,ENCODE_("SELECT A EMPTY FD\n"));
				};
			};
		};
		/*things after epoll_wait*/
		if(timeout_connect!=NULL)
		{
			fake_connect.fd=timeout_connect->fd;
			timeout_connect=&fake_connect;
			timeout_connect=hash_Get(connect_list,&timeout_connect);
			if(timeout_connect!=NULL)
			{
				time(&now_time);
				timeout_connect=*(HTTP_CONNECT**)timeout_connect;
				ERROR_OUT_(stderr,ENCODE_("TIMEOUT HAD BEEN SET\n"));
				if((((now_time-timeout_connect->last_op)>config->keep_alive)&&(timeout_connect->op_done==TRUE_))||((now_time-timeout_connect->last_op)>config->timeout))
				{
					/*Keep-alive timeout or Connect timeout*/
					timeout_prev=dchain_Prev(timeout_connect);
					ERROR_OUT2_(stderr,ENCODE_("CONNECT TIMEOUT\n"));
					do
					{
						current_connect=timeout_connect;
						timeout_connect=dchain_Next(timeout_connect);

						if(current_connect->mod->mod_Close(current_connect->mod->share,current_connect)&WORK_CLOSEPORT_)
						{
							do
							{
								close_retry=FALSE_;
								if(current_connect->mod->mod_Closeport(current_connect->mod->share,current_connect,&addport)>0)close_retry=TRUE_;
								ERROR_OUT_(stderr,ENCODE_("CLOSE NEW_FD IS %d\n"),addport.fd);
								if(addport.fd<0)break;
								fake_connect.fd=addport.fd;
								close_connect=&fake_connect;
								close_connect=*(HTTP_CONNECT**)hash_Get(connect_list,&close_connect);
								if(close_connect!=NULL)event_Delete(connect_list,connect_chain,close_connect,epoll_fd);
								else goto fail_return;
							}while(close_retry==TRUE_);
						};

						close(current_connect->fd);
						event_Delete(connect_list,connect_chain,current_connect,epoll_fd);
					}while(timeout_connect!=NULL);
					timeout_connect=timeout_prev;
				};
			}
			else
			{
				timeout_connect=dchain_Tail(connect_chain);
			};
		}
		else
		{
			timeout_connect=dchain_Tail(connect_chain);
		};
	};
fail_return:
	ERROR_PRINT_;
	exit(1);
};

void sig_Int(int sig)
{
	ERROR_OUT2_(stderr,ENCODE_("\nSIGINT RECV\n"));
	exit(0);
};

int main(int argc,char *argv[])
{
	FILE *fp;
	IO_CONFIG io_config;
	
	int listenfd;
	struct sockaddr_in server;

	/*epoll init*/
	int epoll_fd;
	C_HASH connect_list;
	C_DCHAIN connect_chain;

	/*some options*/
	int sock_op;

	SET_LOCALE_("");
	if(argc>1&&argc<3)
	{
		
		fp=fopen(argv[1],"r");
		io_Init(&io_config,fp);
		fclose(fp);
	}
	else
	{
		fp=fopen("resource/config.xml","r");
		io_Init(&io_config,fp);
		fclose(fp);
	};

	signal(SIGPIPE,SIG_IGN);
	signal(SIGINT,&sig_Int);
	if((listenfd=socket(AF_INET,SOCK_STREAM,0))==-1)goto fail_return;
	sock_op=1;
	setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&sock_op,sizeof(sock_op));
	if((bind(listenfd,(struct sockaddr*)&io_config.addr,sizeof(server)))==-1)goto fail_return;
	if(listen(listenfd,io_config.pool_length)==-1)goto fail_return;
	/*fd_Setnonblocking(listenfd);*/
	
	/*now create an epoll pool*/
	epoll_fd=epoll_create(io_config.pool_length);
	hash_Create(&connect_list,&connect_Tinyhash,&connect_Ensure,sizeof(HTTP_CONNECT*),(UINT_)HASH_SPACE_+1);
	dchain_Create(&connect_chain,sizeof(HTTP_CONNECT));

	/*add the listen port to list*/
	event_Add(&connect_list,&connect_chain,epoll_fd,listenfd,EPOLLIN,NULL);
	ERROR_OUT_(stderr,ENCODE_("the listen fd is%d\n"),listenfd);

	/*start epoll loop*/
	epoll_Loop(&connect_list,&connect_chain,epoll_fd,&io_config,listenfd);

	fail_return:
	ERROR_PRINT_;
	return -1;
};
