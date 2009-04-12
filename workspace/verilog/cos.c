#include <stdio.h>
#include <math.h>

#define pi 3.1415926f

int main(int argc,char *argv[])
{
	double i;
	int j;
	
	printf("\n");
	for(i=0,j=0;i<pi/2;i+=pi/255/2,++j)printf("cos_list[%d]=2'h%x;\n",j,((int)(cos(i)*255))&255);
	printf("\ntotal %d j\n",j);
	return 0;
};
