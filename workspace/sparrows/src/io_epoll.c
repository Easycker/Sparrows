#include "io_epoll.h"

IO_CONFIG* io_Config(IO_CONFIG *config,FILE *fp)
{
	XML_DOC doc;
	XML_NODE *config_root;
	C_STRING cache;

	cache=string_Create();
	xml_Open(&doc,fp,STORE_ALL);
	config_root=xml_Nodebyname(ENCODE_("io_epoll"),doc.root);
	xml_Storedata(&cache,xml_Nodebyname(ENCODE_("http_port"),config_root),&doc);
	config->http_port=STRTOUL_(cache,NULL,0);
	xml_Storedata(&cache,xml_Nodebyname(ENCODE_("io_buf"),config_root),&doc);
	config->io_buf=STRTOUL_(cache,NULL,0);
	
	string_Drop(&cache);
	xml_Close(&doc);
};

UINT_ connect_Tinyhash(HTTP_CONNECT *connect)
{
	uint32_t hash=0;
	uint32_t x=0;
	int i;
	char *op;
	size_t len;

	len=sizeof(connect->fd);
	op=&connect->fd;
	for(i=0;i<len;++i)
	{
		hash=(hash<<4)+(*op++);
		if((x=hash&0xF0000000L)!=0)
		{
			hash^= (x>>24);
			hash&=~x;
		};
	};
	return (hash&0x0000ffff);
};


int main(int argc,char *argv[])
{
	/*a mpm using epoll*/
	FILE *fp;
	IO_CONFIG io_config;
	RESPONDER_CONFIG responder_config;
	
	int listenfd;
	struct sockaddr_in server;

	C_HASH connect_list;

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
		return 0;
	};

	if((listenfd=socket(AF_INET,SOCK_STREAM,0))==-1)goto fail_return;
	server.sin_family=AF_INET;
	server.sin_port=htons(io_config.http_port);
	server.sin_addr.s_addr=htonl(INADDR_ANY);
	if((bind(listenfd,(struct sockaddr*)&server,sizeof(server)))==-1)goto fail_return;
	if(listen(listenfd,io_config.stable_process)==-1)goto fail_return;

};
