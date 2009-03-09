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
	xml_Storedata(&cache,xml_Nodebyname(ENCODE_("cache_port"),config_root),&doc);
	config->cache_port=STRTOUL_(cache,NULL,0);
	xml_Storedata(&cache,xml_Nodebyname(ENCODE_("resv_buf"),config_root),&doc);
	config->resv_buf=STRTOL_(cache,NULL,0);
	
	string_Drop(cache);
	xml_Close(&doc);
};

void child_Main(IO_CONFIG *config,int listenfd)
{
	int fp;
	socklen_t len;
	struct sockaddr_in client;
	struct sockaddr_in server;
	int client_fd;
	int server_fd;
	CHAR_ s_char;
	C_ARRAY char *buf_ansi;
	int resv_len;
	int request_len;
	C_STRING buf;
	char end_char;

	len=sizeof(client);
	PRINTF_(ENCODE_("PID %d STARTED\n"),getpid());
	while(1)
	{
		/*using file lock*/
		fp=creat("/tmp/io_lock",0);
		if(fp<0)
		{
			/*sleep(1);*/
			/*PRINTF_(ENCODE_("OTHER FILE IS USING THE LOCK,I AM PID %d\n"),getpid());*/
		}
		else
		{
			PRINTF_(ENCODE_("PID %d GOT THE LOCK,START TO WORK\n"),getpid());
			if((client_fd=accept(listenfd,(struct sockaddr*)&client,&len))==-1)
			{
				/*error here,continue,*/
				unlink("/tmp/io_lock");
				PRINTF_(ENCODE_("FOUND AN ERROR\n"));
			}
			else
			{
				unlink("/tmp/io_lock");
				PRINTF_(ENCODE_("GOT A REQUEST,MY PID IS %d\n"),getpid());
				/*check the request*/
				buf_ansi=array_Create(sizeof(char));
				buf_ansi=array_Resize(buf_ansi,config->resv_buf);
				request_len=0;
				do
				{
					resv_len=read(client_fd,((char*)buf_ansi)+request_len,config->resv_buf);
					request_len+=resv_len;
					array_Head(buf_ansi)->array_length+=resv_len;
					/*PRINTF_(ENCODE_("RESVING,THE REQUEST_LEN IS:%d,THE RESV_LEN IS:%d,NEW SIZE IS %d\n"),request_len,resv_len,array_Length(buf_ansi)+request_len);*/
					buf_ansi=array_Resize(buf_ansi,array_Length(buf_ansi)+request_len+1);
				}while((int)resv_len>=(int)(config->resv_buf));
				end_char='\0';
				buf_ansi=array_Append(buf_ansi,&end_char);

				buf=string_Create();
				string_Ansitowide(&buf,buf_ansi);
				array_Drop(buf_ansi);

				PRINTF_(ENCODE_("RESV DATA:\n%S\n"),buf);
				send(client_fd,"Hello!\n",8,0);
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

	/*
	PRINTF_(ENCODE_("the max process is:%lu\n"),config.max_process);
	PRINTF_(ENCODE_("the stable process is:%lu\n"),config.stable_process);
	PRINTF_(ENCODE_("the http port is:%lu\n"),config.http_port);
	PRINTF_(ENCODE_("the io_port is:%lu\n"),config.io_port);
	*/

	/*create a socket*/
	if((listenfd=socket(AF_INET,SOCK_STREAM,0))==-1)goto fail_return;
	server.sin_family=AF_INET;
	server.sin_port=htons(config.http_port);
	server.sin_addr.s_addr=htonl(INADDR_ANY);
	if((bind(listenfd,(struct sockaddr*)&server,sizeof(server)))==-1)goto fail_return;
	if(listen(listenfd,config.stable_process)==-1)goto fail_return;

	/*now fork the childs*/
	for(i=0;i<config.stable_process;++i)
	{
		pid=fork();
		if(pid==0)
		{
			child_Main(&config,listenfd);
			exit(0);
		};
	};
	i=0;
	while(1)
	{
		sleep(1);
	};

	return 0;
	fail_return:
	/*
	PRINTF_(ENCODE_("PROGRAM FAILED\n"));
	if(errno==EACCES)PRINTF_(ENCODE_("EACCES\n"));
	if(errno==EADDRINUSE)PRINTF_(ENCODE_("EADDRINUSE\n"));
	if(errno==EBADF)PRINTF_(ENCODE_("EBADF\n"));
	if(errno==EINVAL)PRINTF_(ENCODE_("EINVAL\n"));
	if(errno==ENOTSOCK)PRINTF_(ENCODE_("ENOTSOCK\n"));
	*/
	return -1;
};
