#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

int main()
{
	int soc;
	struct sockaddr_in my_addr;
	struct sockaddr_in re_addr;
	int re_soc;
	socklen_t soc_length;
	int n;
	char buf[2048];

	my_addr.sin_family=AF_INET;
	my_addr.sin_port=htons(8802);
	my_addr.sin_addr.s_addr=INADDR_ANY;
	soc_length=sizeof(my_addr);
	soc=socket(AF_INET,SOCK_STREAM,0);
	bind(soc,&my_addr,sizeof(my_addr));
	listen(soc,100);

	for(;;)
	{
		re_soc=accept(soc,&re_addr,&soc_length);
		
		n=read(re_soc,buf,20);
		buf[2047]='\0';
		printf(buf);
		
		send(re_soc,"Hello world!\n",19,0);
	};
};