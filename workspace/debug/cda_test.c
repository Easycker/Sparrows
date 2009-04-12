#include <stdio.h>
#include <cda/c_array.h>
#include <cda/c_mem.h>
#include <time.h>

int main()
{
	void *malloc_space;
	C_POOL my_space;
	int i;
	clock_t my_time;

	my_time=clock();
	for(i=0;i<5000000;++i)malloc_space=malloc(8);
	printf("first time:%f\n",clock()-my_time);

	my_time=clock();
	pool_Create(&my_space,8);
	for(i=0;i<5000000;++i)malloc_space=pool_Malloc(&my_space);
	printf("sendeo time:%f\n",clock()-my_time);
};