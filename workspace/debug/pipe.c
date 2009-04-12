#include <stdio.h>
#include <unistd.h>

int main()
{
	int f_de[2];
	char buf[512];
	int i;

	pipe(f_de);
	if(fork()==0)
	{
		dup2(f_de[1],fileno(stdout));
		close(f_de[1]);
		close(f_de[0]);
		printf("hello");
	}
	else
	{
		i=read(f_de[0],buf,512);
		printf("recv the child:%d bytes\n,they are: %s",i,buf);
	};
	return 0;
};
