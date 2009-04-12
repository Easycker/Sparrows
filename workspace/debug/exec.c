#include <stdio.h>
#include <unistd.h>

int main(int argc,char *argv[])
{
	if(argc>1)
	{
		execvp(argv[1],argv+1);
		printf("failed");
		exit(1);
	};
	return 0;
};
