#include <stdio.h>

int main()
{
	/*
	unsigned int i;
	i-=1;
	printf("a hash may be:%u\n",((i-1)>>16)*8);
	*/
	
	unsigned  int  hash  =   0 ;
        unsigned  int  x     =   0 ;
	char str[]="/pic/.png";
	char *op=str;

         while  ( * op)
          {
                hash  =  (hash  <<   4 )  +  ( * op ++ );
                 if  ((x  =  hash  &   0xF0000000L )  !=   0 )
                  {
                        hash  ^=  (x  >>   24 );
                        hash  &=   ~ x;
                }
	  }
	  printf("hash is:%u\n",(char)hash&255);
	
	return 0;
};