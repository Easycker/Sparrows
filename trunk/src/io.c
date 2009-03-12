#include "io.h"

IO_CONFIG* io_Config(IO_CONFIG *config,FILE *fp)
{
	XML_DOC doc;
	XML_NODE *config_root;
	XML_NODE *node;
	C_STRING cache;

	cache=string_Create();
	xml_Open(&doc,fp,STORE_ALL);
	config_root=xml_Nodebyname(ENCODE_("mod_io"),doc.root);

	xml_Storedata(&cache,xml_Nodebyname(ENCODE_("max_process"),config_root),&doc);
	config->max_process=STRTOUL_(cache,NULL,0);
	xml_Storedata(&cache,xml_Nodebyname(ENCODE_("stable_process"),config_root),&doc);
	config->stable_process=STRTOUL_(cache,NULL,0);
	xml_Storedata(&cache,xml_Nodebyname(ENCODE_("http_port"),config_root),&doc);
	config->http_port=STRTOUL_(cache,NULL,0);
	config->cache_adr=string_Create_Ex(xml_Storedata(&cache,xml_Nodebyname(ENCODE_("cache_adr"),config_root),&doc));
	config->lock_file=string_Create_Ex(xml_Storedata(&cache,xml_Nodebyname(ENCODE_("lock_file"),config_root),&doc));
	xml_Storedata(&cache,xml_Nodebyname(ENCODE_("cache_port"),config_root),&doc);
	config->cache_port=STRTOUL_(cache,NULL,0);
	xml_Storedata(&cache,xml_Nodebyname(ENCODE_("resv_buf"),config_root),&doc);
	config->resv_buf=STRTOL_(cache,NULL,0);
	
	string_Drop(cache);
	xml_Close(&doc);
};

LOCK_T* lock_Init(LOCK_T *lock,C_ARRAY CHAR_* const filename)
{
	lock->file=array_Create(sizeof(char));
	if(string_Widetoansi(&lock->file,filename)==NULL)goto fail_return;
	return lock;

	fail_return:
	array_Drop(lock->file);
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

void child_Main(IO_CONFIG *config,int listenfd,LOCK_T *child_lock)
{
	int fp;
	socklen_t len;
	struct sockaddr_in client;
	int client_fd;
	C_STRING buf;
	HTTP_REQUEST request;
	BOOL_ lock;

	len=sizeof(client);
	PRINTF_(ENCODE_("PID %d STARTED\n"),getpid());
	while(1)
	{
		/*using file lock*/
		lock=lock_Get(child_lock);
		if(lock==FALSE_)
		{
			sleep(1);
			/*PRINTF_(ENCODE_("OTHER FILE IS USING THE LOCK,I AM PID %d\n"),getpid());*/
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
				buf=string_Create();
				httpd_Nrecv(&buf,client_fd,config->resv_buf,0);
				PRINTF_(ENCODE_("RESV DATA:\n%S\n"),buf);
				request_Create(&request);
				if(request_Analysis(&request,buf)==NULL)PRINTF_(ENCODE_("FAILED\n"));
				PRINTF_(ENCODE_("type:%d,path:%S\n"),request.type,request.path);
				send(client_fd,"Hello!\n",8,0);
				string_Drop(buf);
				close(client_fd);
			};
		};
	};
};

int main(int argc,char *argv[])
{
	/*this is a process-base mpm*/
	FILE *fp;
	IO_CONFIG config;
	UINT_ i;
	pid_t pid;
	LOCK_T lock;

	int listenfd;
	struct sockaddr_in server;

	if(argc>1&&argc<3)
	{
	fp=fopen(argv[1],"r");
	io_Config(&config,fp);
	fclose(fp);
	}
	else
	{
		PRINTF_(HOWTO_);
		return 0;
	};

	/*create a socket*/
	if((listenfd=socket(AF_INET,SOCK_STREAM,0))==-1)goto fail_return;
	server.sin_family=AF_INET;
	server.sin_port=htons(config.http_port);
	server.sin_addr.s_addr=htonl(INADDR_ANY);
	if((bind(listenfd,(struct sockaddr*)&server,sizeof(server)))==-1)goto fail_return;
	if(listen(listenfd,config.stable_process)==-1)goto fail_return;

	/*now fork the childs*/
	lock_Init(&lock,config.lock_file);
	for(i=0;i<config.stable_process;++i)
	{
		pid=fork();
		if(pid==0)
		{
			child_Main(&config,listenfd,&lock);
			break;
		};
	};
	i=0;
	while(1)
	{
		sleep(1);
	};

	return 0;
	fail_return:
	PRINTF_(ENCODE_("somewhere error?\n"));

	return -1;
};
