#include <stdio.h>

int main()
{
	unsigned int a;
	unsigned char b;
	unsigned int c;

	a=198;
	b=a;
	printf("%d\n",b);
	((char*)&c)[3]=b;

	printf("%d\n",((char*)&c)[3]);

	return 0;
};