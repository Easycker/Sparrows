#include "io.h"

IO_CONFIG* io_Init(IO_CONFIG *config,FILE *fp)
{
	XML_DOC doc;
	XML_NODE *doc_root;
	XML_NODE *config_root;
	C_STRING cache;
	HOST_TYPE host;
	UINT_ i;
	UINT_ l;
	MOD_T mod;

	cache=string_Create();
	if(cache==NULL)
	{
		ERROR_OUT_(stderr,"MEMORY LEAK\n");
		exit(-1);
	};
	if(xml_Open(&doc,fp,STORE_ALL)==NULL)ERROR_EXIT_;
	doc_root=xml_Nodebyname(ENCODE_("config"),doc.root);
	if(doc_root==NULL)ERROR_EXIT_;
	if(xml_Storedata(&cache,xml_Nodebyname(ENCODE_("http_interface"),doc_root),&doc)==NULL)ERROR_EXIT_;
	if(!STRCMP_(cache,ENCODE_("0")))config->addr.sin_addr.s_addr=htonl(INADDR_ANY);
	else
	if(inet_aton(cache,&config->addr.sin_addr<0))ERROR_EXIT_;

	config->addr.sin_family=AF_INET;
	if(xml_Storedata(&cache,xml_Nodebyname(ENCODE_("http_port"),doc_root),&doc)==NULL)ERROR_EXIT_;
	config->addr.sin_port=htons(STRTOUL_(cache,NULL,0));
	if(xml_Storedata(&cache,xml_Nodebyname(ENCODE_("io_buf"),doc_root),&doc)==NULL)ERROR_EXIT_;
	config->io_buf=STRTOUL_(cache,NULL,0);
	if(xml_Storedata(&cache,xml_Nodebyname(ENCODE_("pool_length"),doc_root),&doc)==NULL)ERROR_EXIT_;
	config->pool_length=STRTOUL_(cache,NULL,0);
	if(xml_Storedata(&cache,xml_Nodebyname(ENCODE_("max_header"),doc_root),&doc)==NULL)ERROR_EXIT_;
	config->max_head=STRTOUL_(cache,NULL,0);
	if(xml_Storedata(&cache,xml_Nodebyname(ENCODE_("pool_timeout"),doc_root),&doc)==NULL)ERROR_EXIT_;
	config->pool_timeout=STRTOUL_(cache,NULL,0);
	if(xml_Storedata(&cache,xml_Nodebyname(ENCODE_("keep_alive"),doc_root),&doc)==NULL)ERROR_EXIT_;
	config->keep_alive=STRTOUL_(cache,NULL,0);
	if(xml_Storedata(&cache,xml_Nodebyname(ENCODE_("timeout"),doc_root),&doc)==NULL)ERROR_EXIT_;
	config->timeout=STRTOUL_(cache,NULL,0);

	/*now read the config of virtual host*/
	config->host_list=array_Create(sizeof(HOST_TYPE));
	doc_root=xml_Nodebyname(ENCODE_("hosts"),doc_root);
	if(doc_root==NULL)ERROR_EXIT_;
	ERROR_OUT_(stderr,ENCODE_("CHILDS:%lu\n"),xml_Nodechilds(doc_root));
	for(l=0;l<xml_Nodechilds(doc_root);++l)
	{
		if(xml_Storedata(&cache,xml_Nodebyname(ENCODE_("regex"),xml_Nodebyindex(l,doc_root)),&doc)==NULL)ERROR_EXIT_;
		if(array_Length(cache)<2)ERROR_EXIT_;
		array_Remove(&cache,&cache[0]);
		array_Remove(&cache,&cache[STRLEN_(cache)-1]);
		if(regcomp(&host.preg,cache,0)<0)ERROR_EXIT_;

		/*here is mod loader*/

		config_root=xml_Nodebyindex(l,doc_root);
		if(config_root==NULL)ERROR_EXIT_;
		host.mod_config.mod_table=array_Create(sizeof(MOD_T));
		if(host.mod_config.mod_table==NULL)ERROR_EXIT_;
		/*read the mods conf*/
		for(i=0;i<xml_Nodechilds(xml_Nodebyname(ENCODE_("conf"),config_root));++i)
		{
			/*clean up the mod_t*/
			ERROR_OUT_(stderr,ENCODE_("XML NODE CHILDS:%lu\n"),xml_Nodechilds(xml_Nodebyname(ENCODE_("conf"),config_root)));

			xml_Nodename(&cache,xml_Nodebyindex(i,xml_Nodebyname(ENCODE_("conf"),config_root)));
			if(array_Length(cache)<3)ERROR_EXIT_;
			if(!strcmp(cache,STATIC_NAME))
			{
				mod.mod_Init=(void*)&static_Init;
				mod.mod_Select=&static_Select;
				mod.mod_Work=&static_Work;
				mod.mod_Addport=&static_Addport;
				mod.mod_Closeport=&static_Closeport;
				mod.mod_Close=&static_Close;
				mod.mod_Unload=&static_Unload;
			}
			else if(!strcmp(cache,CGI_NAME))
			{
				mod.mod_Init=(void*)&cgi_Init;
				mod.mod_Select=&cgi_Select;
				mod.mod_Work=&cgi_Work;
				mod.mod_Addport=&cgi_Addport;
				mod.mod_Closeport=&cgi_Closeport;
				mod.mod_Close=&cgi_Close;
				mod.mod_Unload=&cgi_Unload;
			}
			else if(!strcmp(cache,PROXY_NAME))
			{
				mod.mod_Init=(void*)&proxy_Init;
				mod.mod_Select=&proxy_Select;
				mod.mod_Work=&proxy_Work;
				mod.mod_Addport=&proxy_Addport;
				mod.mod_Closeport=&proxy_Closeport;
				mod.mod_Close=&proxy_Close;
				mod.mod_Unload=&proxy_Unload;
			}
			else if(!strcmp(cache,REWRITE_NAME))
			{
				mod.mod_Init=(void*)&rewrite_Init;
				mod.mod_Select=&rewrite_Select;
				mod.mod_Work=&rewrite_Work;
				mod.mod_Addport=&rewrite_Addport;
				mod.mod_Closeport=&rewrite_Closeport;
				mod.mod_Close=&rewrite_Close;
				mod.mod_Unload=&rewrite_Unload;
			}

			xml_Parmbyname(&cache,ENCODE_("arg"),xml_Nodebyindex(i,xml_Nodebyname(ENCODE_("conf"),config_root)));
			if(array_Length(cache)<3)ERROR_EXIT_;
			array_Remove(&cache,&cache[0]);
			array_Remove(&cache,&cache[STRLEN_(cache)-1]);

			if(mod.mod_Init!=NULL)mod.share=(void*)mod.mod_Init(config,cache);
			else ERROR_EXIT_;
			if(mod.share==NULL)ERROR_EXIT_;

			array_Append(&host.mod_config.mod_table,&mod);
		};

		/*a host had been read,append it*/
		array_Append(&config->host_list,&host);
		ERROR_OUT_(stderr,ENCODE_("ADD ONE HOST\n"));
	};
	string_Drop(&cache);
	xml_Close(&doc);

	return config;
fail_return:
	/*ERROR_PRINT_;*/
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

HTTP_CONNECT* event_Add(C_HASH *table,C_DLIST *list,struct event_pool *pool,int fd,uint32_t event,MOD_T *mod)
{
	HTTP_CONNECT connect;
	HTTP_CONNECT *inserted_con;

	if(fd_Setnonblocking(fd)==-1)ERROR_EXIT_;
	
	if(eventadd(pool,fd,event)!=-1)
	{
		connect.fd=fd;
		connect.mod=mod;
		if(time(&connect.last_op)<0)ERROR_EXIT_;
		connect.op_done=FALSE_;
		if(mod!=NULL)
		{
			if(list->dlist_head==NULL)inserted_con=dlist_Append(&connect,list);
			else inserted_con=dlist_Insert(&connect,list->dlist_head,list);
			hash_Append(table,&inserted_con);
		}
		else
		{
			inserted_con=malloc(sizeof(HTTP_CONNECT));
			if(inserted_con==NULL)ERROR_EXIT_;
			memcpy(inserted_con,&connect,sizeof(HTTP_CONNECT));
			hash_Append(table,&inserted_con);
		};
		ERROR_OUT_(stderr,ENCODE_("ADD A FD,IT'S %d\n"),connect.fd);
	}
	else
	{
		ERROR_OUT2_(stderr,ENCODE_("ERROR ON ADD EVENT\n"));
		ERROR_EXIT_;
	};
	return inserted_con;
fail_return:
	return NULL;
};

void event_Mod(struct event_pool *pool,int fd,uint32_t event)
{
	ERROR_OUT_(stderr,ENCODE_("GOING TO MOD THE ID:%d\n"),fd);
	if(eventmod(pool,fd,event)<0)ERROR_EXIT_;
fail_return:
	return;
};

void event_Active(C_HASH *table,C_DLIST *connect_list,HTTP_CONNECT *connect)
{
	HTTP_CONNECT *next;

	if(connect_list->dlist_length>1)
	{
		ERROR_OUT_(stderr,ENCODE_("ACTIVE START\n"));
		next=(HTTP_CONNECT*)dlist_Next(connect);
		if(connect_list->dlist_head!=connect)
		{
			dlist_Next(dlist_Prev(connect))=dlist_Next(connect);
			if(dlist_Next(connect)!=NULL)dlist_Prev(dlist_Next(connect))=dlist_Prev(connect);
			else connect_list->dlist_tail=dlist_Prev(connect);
			dlist_Next(connect)=connect_list->dlist_head;
			dlist_Prev(connect_list->dlist_head)=connect;
			dlist_Prev(connect)=NULL;
			connect_list->dlist_head=connect;
		};
	}
	time(&(connect->last_op));
};

C_HASH* event_Delete(C_HASH *table,C_DLIST *list,HTTP_CONNECT *connect,struct event_pool *pool)
{
	/*struct epoll_event io_event;*/

	ERROR_OUT_(stderr,ENCODE_("GOING TO DEL THE %d\n"),connect->fd);
	
	if(eventdel(pool,connect->fd)<0)
	{
		ERROR_OUT_(stderr,ENCODE_("ERROR ON EVENT DELETE\n"));
		/*ERROR_PRINT_;*/
	};
	hash_Remove(table,&connect);
	dlist_Remove(connect,list);
	
	return table;
};

int event_Loop(C_HASH *connect_table,C_DLIST *connect_list,struct event_pool *pool,IO_CONFIG *config,int listenfd)
{
	struct event_type *events;
	int wait_fd;
	uint32_t event_t;
	HTTP_CONNECT fake_connect;
	HTTP_CONNECT *current_connect;
	HTTP_CONNECT *timeout_connect;
	HTTP_CONNECT *close_connect;
	int client_fd;
	struct sockaddr_in client;
	int len;
	int i;
	int state;
	PORT_APPLY addport;
	BOOL_ retry;
	BOOL_ close_retry;

	MOD_T head_mod;
	time_t now_time;

	sigset_t sigs;

	/*init the head_mod*/
	head_mod.share=head_Init((config->max_head)/2,config->max_head,config->host_list);
	if(head_mod.share==NULL)ERROR_EXIT_;
	head_mod.mod_Work=&head_Work;
	head_mod.mod_Close=&head_Close;

	/*catch the sigint signal*/
	if(sigemptyset(&sigs)<0)ERROR_EXIT_;
	if(sigaddset(&sigs,SIGINT)<0)ERROR_EXIT_;

	/*prepare a epoll loop*/
	len=sizeof(client);
	events=(struct event_type*)malloc(sizeof(struct event_type)*(config->pool_length));
	/*timeout_connect use for mark a timeouted connect*/
	timeout_connect=NULL;
	/*loop until recv a sigint*/
	while(1)
	{
		/*wait for events*/
		/*wait_fd=epoll_wait(epoll_fd,events,config->pool_length,config->pool_timeout);*/
		wait_fd=eventwait(pool,events,config->pool_length,config->pool_timeout);
		/*events!i got it!*/
		if(wait_fd>0)
		{
			for(i=0;i<wait_fd;++i)
			{
				ERROR_OUT_(stderr,ENCODE_("TOTAL %d events exist,DOING the %d\n"),wait_fd,i);
				fake_connect.fd=events[i].fd;
				ERROR_OUT_(stderr,ENCODE_("READY TO GET,FD IS %d\n"),fake_connect.fd);
				current_connect=&fake_connect;
				/*is the event exits?*/
				current_connect=(HTTP_CONNECT*)hash_Get(connect_table,&current_connect);
				/*it exits*/
				if(current_connect!=NULL)
				{
					/*set the event*/
					state=0;
					current_connect=*(HTTP_CONNECT**)current_connect;
					current_connect->event=0;
					/*it can read or write? or both?*/
					if(events[i].event|EVENT_IN)current_connect->event|=WORK_INPUT_;
					if(events[i].event|EVENT_OUT)current_connect->event|=WORK_OUTPUT_;
					ERROR_OUT_(stderr,ENCODE_("A EXIST EVENT,IT'S ID IS:%d\n"),current_connect->fd);
					/*is it the listenning event?*/
					if(current_connect->fd==listenfd)
					{
						ERROR_OUT_(stderr,ENCODE_("A NEW CONNECT GOING TO BE CREATE\n"));
						/*accept it*/
						while(1)
						{
							client_fd=accept(listenfd,(struct sockaddr*)&client,(socklen_t*)&len);
							/*accept failed? impossible! but it really happend however, it may cause by too many connecttions*/
							if(client_fd<0)
							{
								ERROR_OUT2_(stderr,ENCODE_("ERROR ON ACCEPT\n"));
								/*perror("");*/
								if(errno==EMFILE)ERROR_EXIT_;
								if(errno==ENFILE)ERROR_EXIT_;
								break;
							};
							/*looks fine, add it to the event pool, be careful, the accept may not be done yet!*/
							current_connect=event_Add(connect_table,connect_list,pool,client_fd,EVENT_IN,&head_mod);
							ERROR_OUT_(stderr,ENCODE_("NEW CONNECT FINISH\n"));
							event_Active(connect_table,connect_list,current_connect);
						}
					}
					/*not a listenning event, must be a accepted event or processing event*/
					else
					{
						ERROR_OUT_(stderr,ENCODE_("EVERYTING LOOKS OK,LET'S WORK,FD IS %d\n"),current_connect->fd);
						if(current_connect->mod==&head_mod)ERROR_OUT_(stderr,ENCODE_("IS HEAD MOD\n"));
						state=0;
						/*must be a mod work for it*/
						state=current_connect->mod->mod_Work(current_connect->mod->share,current_connect);
						/*today's work is finished, what the result?*/
						do
						{
							retry=FALSE_;
							/*it's time to close the connecttion?*/
							if(state&WORK_CLOSE_)
							{
								/*delete the epoll event but not close the port*/
								ERROR_OUT_(stderr,ENCODE_("CONNECT IS DOWN\n"));
								if(current_connect->mod->mod_Close(current_connect->mod->share,current_connect)&WORK_CLOSEPORT_)
								{
									/*there must be more connecttion to be close, close them all*/
									do
									{
										close_retry=FALSE_;
										if(current_connect->mod->mod_Closeport(current_connect->mod->share,current_connect,&addport)>0)close_retry=TRUE_;
										ERROR_OUT_(stderr,ENCODE_("CLOSE NEW_FD IS %d\n"),addport.fd);
										if(addport.fd<0)break;
										fake_connect.fd=addport.fd;
										close_connect=&fake_connect;
										close_connect=(HTTP_CONNECT*)hash_Get(connect_table,&close_connect);
										if(close_connect!=NULL)
										{
											close_connect=*(HTTP_CONNECT**)close_connect;
											close(close_connect->fd);
											event_Delete(connect_table,connect_list,close_connect,pool);
										}
										else ERROR_EXIT_;
									}while(close_retry==TRUE_);
								};
								close(current_connect->fd);
								/*timeout_connect=NULL;*/
								event_Delete(connect_table,connect_list,current_connect,pool);
							}
							/*work finished, but keep-alive is enabled, wait a moument for next mission, change the working mod to head_mod*/
							else if(state&WORK_KEEP_)
							{
								ERROR_OUT_(stderr,ENCODE_("KEEP ALIVE\n"));
								event_Active(connect_table,connect_list,current_connect);
								current_connect->op_done=TRUE_;
								current_connect->mod->mod_Close(current_connect->mod->share,current_connect);
								current_connect->mod=&head_mod;
								event_Mod(pool,current_connect->fd,EVENT_IN);
							}
							/*still need some more time? ok*/
							else if(!(state&WORK_GOON_))
							{
								event_t=0;
								if(state&WORK_INPUT_)event_t|=EVENT_IN;
								if(state&WORK_OUTPUT_)event_t|=EVENT_OUT;
								event_Mod(pool,current_connect->fd,event_t);
								event_Active(connect_table,connect_list,current_connect);
							}
							/*nothing happened? ok,let's wait...*/
							else
							{
								/*here goon*/
								event_Active(connect_table,connect_list,current_connect);
							};
							/*apply a new event, even the applier want to close itself*/
							if(state&WORK_NEWPORT_)
							{
								/*a new port here*/
								ERROR_OUT_(stderr,ENCODE_("NEW PORT HERE\n"));
								state=0;
								state=current_connect->mod->mod_Addport(current_connect->mod->share,current_connect,&addport);
								current_connect=event_Add(connect_table,connect_list,pool,addport.fd,EVENT_IN,current_connect->mod);
								retry=TRUE_;
							};
						}while(retry==TRUE_);
					};
				}
				/*it's a empty event, this is common after closing a number of events*/
				else
				{
					ERROR_OUT_(stderr,ENCODE_("SELECT A EMPTY FD\n"));
				};
			};
		};
		/*things after epoll_wait, clean the timeout connecttions*/
		while(connect_list->dlist_length>0)
		{
			timeout_connect=dlist_Tail(connect_list);
			if(timeout_connect!=NULL)
			{
				ERROR_OUT_(stderr,"waiting to delete...\n");
				fake_connect.fd=timeout_connect->fd;
				timeout_connect=&fake_connect;
				timeout_connect=hash_Get(connect_table,&timeout_connect);
				if(timeout_connect!=NULL)
				{
					time(&now_time);
					timeout_connect=*(HTTP_CONNECT**)timeout_connect;
					ERROR_OUT_(stderr,ENCODE_("TIMEOUT HAD BEEN SET\n"));
					if((((now_time-timeout_connect->last_op)>config->keep_alive)&&(timeout_connect->op_done==TRUE_))||((now_time-timeout_connect->last_op)>config->timeout))
					{
						/*Keep-alive timeout or Connect timeout*/
						ERROR_OUT2_(stderr,ENCODE_("CONNECT TIMEOUT\n"));

						current_connect=timeout_connect;
						if(current_connect->mod->mod_Close(current_connect->mod->share,current_connect)&WORK_CLOSEPORT_)
						{
							/*there must be more connecttion to be close, close them all*/
							do
							{
								close_retry=FALSE_;
								if(current_connect->mod->mod_Closeport(timeout_connect->mod->share,current_connect,&addport)>0)close_retry=TRUE_;
								ERROR_OUT_(stderr,ENCODE_("CLOSE NEW_FD IS %d\n"),addport.fd);
								if(addport.fd<0)break;
								fake_connect.fd=addport.fd;
								close_connect=&fake_connect;
								close_connect=(HTTP_CONNECT*)hash_Get(connect_table,&close_connect);
								if(close_connect!=NULL)
								{
									close_connect=*(HTTP_CONNECT**)close_connect;
									close(close_connect->fd);
									event_Delete(connect_table,connect_list,close_connect,pool);
								}
								else ERROR_EXIT_;
							}while(close_retry==TRUE_);
						};

						close(timeout_connect->fd);
						event_Delete(connect_table,connect_list,timeout_connect,pool);
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
			else
			{
				break;
			}
		}
	};
fail_return:
	/*ERROR_PRINT_;*/
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
	struct event_pool pool;

	/*event vars,for event management*/
	C_HASH connect_table;
	C_DLIST connect_list;

	/*some options*/
	int sock_op;

	/*read the arg*/
	if(argc>1&&argc<3)
	{
		
		if((fp=fopen(argv[1],"r"))<=0)
		{
			printf("ERROR: %s not founded,is it really there?\n",argv[1]);
			return 1;
		};
		io_config.config_path=string_Create_Ex(argv[1]);
		if(io_Init(&io_config,fp)==NULL)ERROR_EXIT_;
		fclose(fp);
	}
	else
	{
		/*default config file is /etc/sparrows*/
		if((fp=fopen("/etc/sparrows","r"))<=0)
		{
			printf("ERROR: /etc/sparrows not founded,you should use your own config?\n");
			return 1;
		};
		io_config.config_path=string_Create_Ex("/etc/sparrows");
		if(io_Init(&io_config,fp)==NULL)ERROR_EXIT_;
		fclose(fp);
	};

	/*ignore all error signals*/
	signal(SIGPIPE,SIG_IGN);
	signal(SIGCHLD,SIG_IGN);
	signal(SIGINT,&sig_Int);

	/*start listenning*/
	if((listenfd=socket(AF_INET,SOCK_STREAM,0))==-1)ERROR_EXIT_;
	sock_op=1;
	setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&sock_op,sizeof(sock_op));
	if((bind(listenfd,(struct sockaddr*)&io_config.addr,sizeof(server)))==-1)ERROR_EXIT_;
	if(listen(listenfd,io_config.pool_length)==-1)ERROR_EXIT_;
	fd_Setnonblocking(listenfd);
	
	/*now create an event pool*/
	if(eventinit(&pool)<0)ERROR_EXIT_;
	/*create a hash table and a linklist to manage events*/
	hash_Create(&connect_table,&connect_Tinyhash,&connect_Ensure,sizeof(HTTP_CONNECT*),(UINT_)HASH_SPACE_+1);
	dlist_Create(&connect_list,sizeof(HTTP_CONNECT));

	/*add the listen event to list*/
	event_Add(&connect_table,&connect_list,&pool,listenfd,EVENT_IN,NULL);
	ERROR_OUT_(stderr,ENCODE_("the listen fd is%d\n"),listenfd);

	/*start event loop*/
	event_Loop(&connect_table,&connect_list,&pool,&io_config,listenfd);

	fail_return:
	return -1;
};
