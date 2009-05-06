#include "io_epoll.h"

IO_CONFIG* io_Init(IO_CONFIG *config,FILE *fp)
{
	XML_DOC doc;
	XML_NODE *config_root;
	C_STRING cache;

	cache=string_Create();
	if(cache==NULL)
	{
		ERROR_OUT_("MEMORY LEAK\n");
		exit(-1);
	};
	xml_Open(&doc,fp,STORE_ALL);
	config_root=xml_Nodebyname(ENCODE_("io_epoll"),doc.root);
	xml_Storedata(&cache,xml_Nodebyname(ENCODE_("http_port"),config_root),&doc);
	config->http_port=STRTOUL_(cache,NULL,0);
	xml_Storedata(&cache,xml_Nodebyname(ENCODE_("io_buf"),config_root),&doc);
	config->io_buf=STRTOUL_(cache,NULL,0);
	xml_Storedata(&cache,xml_Nodebyname(ENCODE_("poll_length"),config_root),&doc);
	config->poll_length=STRTOUL_(cache,NULL,0);
	
	string_Drop(&cache);
	xml_Close(&doc);
	return config;
};

UINT_ connect_Tinyhash(HTTP_CONNECT *connect)
{
	/*
	uint32_t hash=0;
	uint32_t x=0;
	int i;
	char *op;
	size_t len;

	len=sizeof(connect->fd);
	op=(char*)&(connect->fd);
	for(i=0;i<len;++i)
	{
		hash=(hash<<4)+(*op++);
		if((x=hash&0xF0000000L)!=0)
		{
			hash^= (x>>24);
			hash&=~x;
		};
	};
	return (hash&HASH_SPACE_);
	*/
	return connect->fd&HASH_SPACE_;
};

BOOL_ connect_Ensure(HTTP_CONNECT *lhs,HTTP_CONNECT *rhs)
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

C_HASH* event_Add(C_HASH *table,int epoll_fd,int fd,uint32_t event,MOD_T *mod)
{
	HTTP_CONNECT connect;
	struct epoll_event io_event;

	memset(&io_event,0,sizeof(struct epoll_event));
	io_event.data.fd=fd;
	io_event.events=event;
	if(fd_Setnonblocking(fd)==-1)goto fail_return;
	
	if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,fd,&io_event)!=-1)
	{
		epoll_ctl(epoll_fd,EPOLL_CTL_ADD,fd,&io_event);
		connect.fd=fd;
		connect.mod=mod;
		hash_Append(table,&connect);
		ERROR_OUT_(stderr,ENCODE_("ADD A FD,IT'S %d\n"),connect.fd);
	}
	else
	{
		ERROR_OUT_(stderr,ENCODE_("ERRNO:%d\n"),errno);
		if(errno==EBADF)ERROR_OUT_(stderr,ENCODE_("THE FD IS ERROR\n"));
		return NULL;
	};
	return table;
fail_return:
	ERROR_OUT_(stderr,ENCODE_("SETTING NONBLOCK FAIL\n"));
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
		ERROR_OUT_(stderr,ENCODE_("MOD ERRNO:%d\n"),errno);
		if(errno==EBADF)ERROR_OUT_(stderr,ENCODE_("THE FD IS ERROR\n"));
		if(errno==ENOENT)ERROR_OUT_(stderr,ENCODE_("NO IN EPOLLFD\n"));
	};
fail_return:
	return;
};

C_HASH* event_Delete(C_HASH *table,HTTP_CONNECT *connect,int epoll_fd,int fd)
{
	struct epoll_event io_event;

	ERROR_OUT_(stderr,ENCODE_("GOING TO DEL THE %d\n"),connect->fd);
	
	memset(&io_event,0,sizeof(struct epoll_event));
	io_event.data.fd=fd;
	if(epoll_ctl(epoll_fd,EPOLL_CTL_DEL,fd,&io_event)==-1)
	{
		ERROR_OUT_(stderr,ENCODE_("ERRNO:%d\n"),errno);
		if(errno==ENOENT)ERROR_OUT_(stderr,ENCODE_("NO IN EPOLLFD\n"));
		if(errno==EBADF)ERROR_OUT_(stderr,ENCODE_("NO A VALID FD\n"));
	};
	hash_Remove(table,connect);
	
	return table;
};

int epoll_Loop(C_HASH *connect_list,int epoll_fd,IO_CONFIG *config,MODMANAGE_CONFIG *mod_config,int listenfd)
{
	struct epoll_event *events;
	int wait_fd;
	HTTP_CONNECT fake_connect;
	HTTP_CONNECT *current_connect;
	int client_fd;
	struct sockaddr_in client;
	struct epoll_event io_event;
	int len;
	int i;
	int state;
	PORT_APPLY addport;
	BOOL_ retry;

	C_STRING recv_data;
	HTTP_REQUEST request;

	MOD_T head_mod;
	/*init the head_mod*/
	head_mod.share=head_Init(512,mod_config->mod_table);
	if(head_mod.share==NULL)goto fail_return;
	head_mod.mod_Work=&head_Work;

	len=sizeof(client);
	events=(struct epoll_event*)malloc(sizeof(struct epoll_event)*(config->poll_length));
	while(1)
	{
		wait_fd=epoll_wait(epoll_fd,events,config->poll_length,-1);
		if(wait_fd>0)
		{
			for(i=0;i<wait_fd;++i)
			{
				ERROR_OUT_(stderr,ENCODE_("TOTAL %d events exist,DOING the %d\n"),wait_fd,i);
				fake_connect.fd=events[i].data.fd;
				ERROR_OUT_(stderr,ENCODE_("READY TO GET,FD IS %d\n"),fake_connect.fd);
				current_connect=(HTTP_CONNECT*)hash_Get(connect_list,&fake_connect);
				ERROR_OUT_(stderr,ENCODE_("HASH GET DONE\n"));
				if(current_connect!=NULL)
				{
					ERROR_OUT_(stderr,ENCODE_("A EXIST EVENT,IT'S ID IS:%d\n"),current_connect->fd);
					if(current_connect->fd==listenfd)
					{
						ERROR_OUT_(stderr,ENCODE_("A NEW CONNECT GOING TO BE CREATE\n"));
						client_fd=accept(current_connect->fd,(struct sockaddr*)&client,&len);
						if(client_fd<0)
						{
							ERROR_OUT_(stderr,ENCODE_("ERROR ON ACCEPT\n"));
							break;
						};
						ERROR_OUT_(stderr,ENCODE_("ACCEPT IS FINISH\n"));
						event_Add(connect_list,epoll_fd,client_fd,EPOLLIN|EPOLLET,&head_mod);
						ERROR_OUT_(stderr,ENCODE_("NEW CONNECT FINISH\n"));
					}
					else
					{
						if(current_connect->mod==NULL)
						{
							ERROR_OUT_(stderr,ENCODE_("AN EMPTY MOD HERE,MUST BE ERROR"));
						};
						ERROR_OUT_(stderr,ENCODE_("EVERYTING LOOKS OK,LET'S WORK,FD IS %d\n"),current_connect->fd);
						if(current_connect->mod==&head_mod)ERROR_OUT_(stderr,ENCODE_("IS HEAD MOD\n"));
						state=current_connect->mod->mod_Work(current_connect->mod->share,current_connect);
						do
						{
							retry=FALSE_;
							if(state&WORK_CLOSE_)
							{
								/*delete the epoll event but not close the port*/
								ERROR_OUT_(stderr,ENCODE_("CONNECT IS DOWN\n"));
								event_Delete(connect_list,current_connect,epoll_fd,current_connect->fd);
							}
							else
							{
								if(state&WORK_INPUT_)
								{
									event_Mod(epoll_fd,current_connect->fd,EPOLLIN|EPOLLET);
								}
								else if(state&WORK_OUTPUT_)
								{
									ERROR_OUT_(stderr,ENCODE_("CHANGE TO OUTPUT\n"));
									event_Mod(epoll_fd,current_connect->fd,EPOLLOUT|EPOLLET);
								}
								else
								{
									ERROR_OUT_(stderr,ENCODE_("GOON WORK\n"));
									/*event_Keep();*/
								};
							};
							if(state&WORK_NEWPORT_)
							{
								/*a new port here,understand?*/
								ERROR_OUT_(stderr,ENCODE_("NEW PORT HERE\n"));
								state=current_connect->mod->mod_Addport(current_connect->mod->share,current_connect,&addport);
								event_Add(connect_list,epoll_fd,addport.fd,EPOLLIN|EPOLLET,current_connect->mod);
								retry=TRUE_;
							};
						}while(retry==TRUE_);
					};
				};
			};
		};
	};
fail_return:
	exit(1);
};

int main(int argc,char *argv[])
{
	/*a mpm using epoll*/
	FILE *fp;
	IO_CONFIG io_config;
	MODMANAGE_CONFIG mod_config;
	
	int listenfd;
	struct sockaddr_in server;

	/*epoll init*/
	int epoll_fd;
	C_HASH connect_list;

	SET_LOCALE_("");
	if(argc>1&&argc<3)
	{
		
		fp=fopen(argv[1],"r");
		modmanage_Init(&mod_config,fp);
		fclose(fp);
		
		fp=fopen(argv[1],"r");
		io_Init(&io_config,fp);
		fclose(fp);
	}
	else
	{
		fp=fopen("resource/config.xml","r");
		modmanage_Init(&mod_config,fp);
		fclose(fp);
		
		fp=fopen("resource/config.xml","r");
		io_Init(&io_config,fp);
		fclose(fp);
	};

	signal(SIGPIPE,SIG_IGN);
	if((listenfd=socket(AF_INET,SOCK_STREAM,0))==-1)goto fail_return;
	server.sin_family=AF_INET;
	server.sin_port=htons(io_config.http_port);
	server.sin_addr.s_addr=htonl(INADDR_ANY);
	if((bind(listenfd,(struct sockaddr*)&server,sizeof(server)))==-1)goto fail_return;
	if(listen(listenfd,io_config.poll_length)==-1)goto fail_return;
	/*fd_Setnonblocking(listenfd);*/
	
	/*now create an epoll pool*/
	epoll_fd=epoll_create(io_config.poll_length);
	hash_Create(&connect_list,&connect_Tinyhash,&connect_Ensure,sizeof(HTTP_CONNECT),(UINT_)HASH_SPACE_+1);

	/*add the listen port to list*/
	event_Add(&connect_list,epoll_fd,listenfd,EPOLLIN,NULL);
	ERROR_OUT_(stderr,ENCODE_("the listen fd is%d\n"),listenfd);

	/*start epoll loop*/
	epoll_Loop(&connect_list,epoll_fd,&io_config,&mod_config,listenfd);

	fail_return:
	ERROR_OUT_(stderr,ENCODE_("SOMEWHERE FAIL\n"));
	return -1;
};
