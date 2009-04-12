#include <stdio.h>

#include <sys/socket.h>

#include <unistd.h>

#include <sys/types.h>

#include <netinet/in.h>

#include <stdlib.h>

#define SERVER_PORT 20000 

#define BUFFER_SIZE 2000

#define REUQEST_MESSAGE "welcome to connect the server.\n"



void usage(char *name)

{

       printf("usage: %s IpAddr\n",name);

}



int main(int argc, char **argv)

{
	char sendchar[]="GET / HTTP/1.1\r\nHost: www.google.com\r\n\r\n";

       int servfd,clifd,length = 0;

       int i;

       struct sockaddr_in servaddr,cliaddr;

       socklen_t socklen = sizeof(servaddr);

       char buf[BUFFER_SIZE];



       if (argc < 2)

       {

              usage(argv[0]);

              exit(1);

       }



       if ((clifd = socket(AF_INET,SOCK_STREAM,0)) < 0)

       {

              printf("create socket error!\n");

              exit(1);

       }

       srand(time(NULL));

       bzero(&cliaddr,sizeof(cliaddr));

       cliaddr.sin_family = AF_INET;

       servaddr.sin_family = AF_INET;

       inet_aton(argv[1],&servaddr.sin_addr);

       servaddr.sin_port = htons(8802);

       if (connect(clifd,(struct sockaddr*)&servaddr, socklen) < 0)

       {

              printf("can't connect to %s!\n",argv[1]);

              exit(1);

       }

for(i=0;i<10;++i)
{
	send(clifd,sendchar,strlen(sendchar),0);

      length = recv(clifd,buf,BUFFER_SIZE,0);

       if (length < 0)

       {

              printf("error comes when recieve data from server %s!",argv[1]);

              

       }

       printf("%s",buf);
};


       close(clifd);

       return 0;

}