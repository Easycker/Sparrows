#ifndef C_ARRAY_H
#define C_ARRAY_H

#include <malloc.h>
#include <string.h>
#include "c_define.h"

#define C_ARRAY

typedef struct array_head
{
	size_t data_size;
	size_t array_length;
	size_t array_space;
	void (*free_fun)(void* tag_data);
}ARRAY_HEAD;

#define array_Head(tag_array) ((ARRAY_HEAD*)(tag_array)-1)

#define array_Create(size_t) (array_Create_Ex(size_t,NULL))

#define array_Length(tag_array) (array_Head(tag_array)->array_length)

/*C_ARRAY array_Create(size_t data_size);*/

/*warning! all C_ARRAY are a address to the pointer of the array*/
C_ARRAY void* array_Create_Ex(size_t data_size,void (*free_fun)(void *tag_data));

C_ARRAY void* array_Resize(C_ARRAY void* tag_array_p,size_t new_size);

C_ARRAY void* array_Append(C_ARRAY void* tag_array_p,const void *in_data);

C_ARRAY void* array_Insert(C_ARRAY void* tag_array_p,const void *in_data,void *in_cur);

C_ARRAY void* array_Remove(C_ARRAY const void* tag_array_p,void *in_cur);

void array_Drop(C_ARRAY void* tag_array_p);

#endif
