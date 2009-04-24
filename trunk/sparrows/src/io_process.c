#include "io_process.h"

IO_CONFIG* io_Config(IO_CONFIG *config,FILE *fp)
{
	XML_DOC doc;
	XML_NODE *config_root;
	C_STRING cache;

	cache=string_Create();
	xml_Open(&doc,fp,STORE_ALL);
	config_root=xml_Nodebyname(ENCODE_("io_process"),doc.root);
	xml_Storedata(&cache,xml_Nodebyname(ENCODE_("max_process"),config_root),&doc);
	config->max_process=STRTOUL_(cache,NULL,0);
	xml_Storedata(&cache,xml_Nodebyname(ENCODE_("stable_process"),config_root),&doc);
	config->stable_process=STRTOUL_(cache,NULL,0);
	xml_Storedata(&cache,xml_Nodebyname(ENCODE_("http_port"),config_root),&doc);
	config->http_port=STRTOUL_(cache,NULL,0);
	xml_Storedata(&cache,xml_Nodebyname(ENCODE_("io_buf"),config_root),&doc);
	config->io_buf=STRTOUL_(cache,NULL,0);
	config->lock_file=string_Create_Ex(xml_Storedata(&cache,xml_Nodebyname(ENCODE_("lock_file"),config_root),&doc));
	
	string_Drop(&cache);
	xml_Close(&doc);
};

LOCK_T* lock_Init(LOCK_T *lock,C_ARRAY CHAR_* const filename)
{
	lock->file=array_Create(sizeof(char));
	if(string_Widetoansi(&lock->file,filename)==NULL)goto fail_return;
	return lock;

	fail_return:
	array_Drop(&lock->file);
	return NULL;
};

BOOL_ lock_Get(LOCK_T *lock)
{
	int fp;

	fp=creat(lock->file,0);
	if(fp<0)return FALSE_;
	return TRUE_;
};

void lock_Free(LOCK_T *lock)
{
	unlink(lock->file);
};

void child_Main(IO_CONFIG *config,RESPONDER_CONFIG *res_config,int listenfd,LOCK_T *child_lock)
{
	int fp;
	socklen_t len;
	struct sockaddr_in client;
	int client_fd;
	C_STRING recv_data;
	HTTP_REQUEST request;
	BOOL_ lock;
	MOD_T *mod;
	HTTP_CONNECT connect;
	char *buf;
	int buf_len;
	int recv_len;
	int state;
	C_ARRAY char *cache_ansi;

	SET_LOCALE_("");
	len=sizeof(client);
	PRINTF_(ENCODE_("PID %d STARTED\n"),getpid());
	buf_len=config->io_buf;
	buf=(char*)malloc(buf_len*sizeof(char));
	while(1)
	{
		/*using file lock*/
		lock=lock_Get(child_lock);
		if(lock==FALSE_)
		{
			sleep(1);
		}
		else
		{
			PRINTF_(ENCODE_("PID %d GOT THE LOCK,START TO WORK\n"),getpid());
			if((client_fd=accept(listenfd,(struct sockaddr*)&client,&len))==-1)
			{
				/*error here,continue,*/
				lock_Free(child_lock);
				PRINTF_(ENCODE_("FOUND AN ERROR\n"));
			}
			else
			{
				lock_Free(child_lock);
				PRINTF_(ENCODE_("GOT A REQUEST,MY PID IS %d\n"),getpid());
				recv_data=string_Create();
				request_Create(&request);
				/*recv and analysis the head*/
				httpd_Nrecv(&recv_data,client_fd,config->io_buf,0);
				request_Head(&request,recv_data);
				mod=mod_Find(res_config,&request);
				if(mod!=NULL)
				{
					connect.fd=client_fd;
					connect.connect_id=mod->mod_Prepare(mod->config,&request);
					if(connect.connect_id!=NULL)
					{
						if(request.type==POST)
						{
							/*it want post?ok*/
							do
							{
								recv_len=read(connect.fd,buf,buf_len);
								state=mod->mod_Recv(connect.connect_id,buf,recv_len);
								if(state<0)break;
							}while(recv_len>=buf_len);
						};
						cache_ansi=array_Create(sizeof(char));
						string_Widetoansi(&cache_ansi,HTTP_OK_);
						state=send(connect.fd,cache_ansi,array_Length(cache_ansi)-1,0);
						do
						{
							recv_len=mod->mod_Send(connect.connect_id,buf,buf_len);
							if(recv_len<=0)break;
							PRINTF_(ENCODE_("OK BEGIN SEND DATA,THE LENGTH IS %d\n"),recv_len);
							state=send(connect.fd,buf,recv_len,0);
						}while(recv_len>=buf_len);
						mod->mod_End(connect.connect_id);
					}
					else
					{
						/*error here*/
						/*send a 404*/
						string_Widetoansi(&cache_ansi,HTTP_NOT_FOUND_);
						state=send(connect.fd,cache_ansi,array_Length(cache_ansi)-1,0);
					};
				}
				else
				{
					/*no mod aviliabe send a 404*/
					string_Widetoansi(&cache_ansi,HTTP_NOT_FOUND_);
					state=send(connect.fd,cache_ansi,array_Length(cache_ansi)-1,0);
				};
				string_Drop(&recv_data);
				close(client_fd);
				request_Free(&request);
			};
		};
	};
	free(buf);
};

void signal_Quit(int sig)
{
	PRINTF_(ENCODE_("SIGNAL QUIT RECEIVED\n"));
	exit(0);
};

void io_Drop(IO_CONFIG *config)
{
	string_Drop(&config->lock_file);
};

int main(int argc,char *argv[])
{
	/*this is a process-base mpm*/
	FILE *fp;
	IO_CONFIG io_config;
	RESPONDER_CONFIG responder_config;
	UINT_ i;
	pid_t pid;
	LOCK_T lock;

	int listenfd;
	struct sockaddr_in server;

	SET_LOCALE_("");
	if(argc>1&&argc<3)
	{
		fp=fopen(argv[1],"r");
		responder_Config(&responder_config,fp);
		fclose(fp);
		fp=fopen(argv[1],"r");
		io_Config(&io_config,fp);
		fclose(fp);
	}
	else
	{
		PRINTF_(HOWTO_,argv[0]);
		return 0;
	};

	signal(SIGINT,signal_Quit);
	/*create a socket*/
	if((listenfd=socket(AF_INET,SOCK_STREAM,0))==-1)goto fail_return;
	server.sin_family=AF_INET;
	server.sin_port=htons(io_config.http_port);
	server.sin_addr.s_addr=htonl(INADDR_ANY);
	if((bind(listenfd,(struct sockaddr*)&server,sizeof(server)))==-1)goto fail_return;
	if(listen(listenfd,io_config.stable_process)==-1)goto fail_return;

	/*now fork the childs*/
	lock_Init(&lock,io_config.lock_file);
	for(i=0;i<io_config.stable_process;++i)
	{
		pid=fork();
		if(pid==0)
		{
			child_Main(&io_config,&responder_config,listenfd,&lock);
			exit(0);
		};
	};
	
	PRINTF_(ENCODE_("\nTHE SERVER START TO RUN,THE LISTEN PORT IS:%u\n"
			"YOU CAN PRESS q TO KILL THE SERVER AT ANY TIME\n"),io_config.http_port);
	while(1)
	{
		if(FGETC_(stdin)==ENCODE_('q'))
		{
			/*free the resoures and quit*/
			close(listenfd);
			unlink(lock.file);
			array_Drop(&lock.file);
			io_Drop(&io_config);
			responder_Drop(&responder_config);
			PRINTF_(ENCODE_("\nNOW QUIT\n"));
			kill(0,SIGTERM);
		};
	};

	return 0;
	fail_return:
	PRINTF_(ENCODE_("somewhere error?(port had benn used?)\n"));
	return -1;
};
