#include "c_array.h"

C_ARRAY void* array_Create_Ex(size_t data_size,void (*free_fun)(void *tag_data))
{
	ARRAY_HEAD *new_array;
	
	new_array=(ARRAY_HEAD*)malloc(sizeof(ARRAY_HEAD));
		if(new_array==NULL)return NULL;
	
	new_array->data_size=data_size;
	new_array->array_length=0;
	new_array->array_space=0;
	new_array->free_fun=free_fun;
	
	return (C_ARRAY void*)(++new_array);
};

C_ARRAY void* array_Resize(C_ARRAY void* tag_array_p,size_t new_size)
{
	ARRAY_HEAD *array_head;
	void **array_op;
	C_ARRAY void* tag_array;

	tag_array=*(void**)tag_array_p;
	array_head=array_Head(tag_array);
	
	if(array_head->array_length<=new_size)
	{
		array_head=(ARRAY_HEAD*)realloc(array_head,(new_size)*(array_head->data_size)+sizeof(ARRAY_HEAD));
			if(array_head==NULL)return NULL;
	}
	else
	{
		if(array_head->free_fun!=NULL)for(array_op=&((void**)tag_array)[new_size];*(array_op-1)!=((void**)tag_array)[array_head->array_length-1];++array_op)array_head->free_fun(*array_op);
		array_head=(ARRAY_HEAD*)realloc(array_head,(new_size)*(array_head->data_size)+sizeof(ARRAY_HEAD));
	};
	array_head->array_space=new_size;
	array_head->array_length=array_head->array_length>array_head->array_space?array_head->array_space:array_head->array_length;
	*(void**)tag_array_p=(C_ARRAY void*)++array_head;

	return (C_ARRAY void*)++array_head;
};

C_ARRAY void* array_Append(C_ARRAY void* tag_array_p,const void *in_data)
{
	ARRAY_HEAD *array_head;
	char *array_op;
	C_ARRAY void* tag_array;

	tag_array=*(void**)tag_array_p;
	array_head=array_Head(tag_array);
	array_op=(char*)tag_array;
	if((array_head->array_length)>=(array_head->array_space))
	{
		if(array_Resize(&array_op,array_head->array_length+1)==NULL)return NULL;
	};
	
	array_head=array_Head(array_op);

	memcpy(array_op+array_head->array_length*array_head->data_size,in_data,array_head->data_size);
	++(array_head->array_length);
	*(void**)tag_array_p=(C_ARRAY void*)array_op;

	return (C_ARRAY void*)array_op;
};

C_ARRAY void* array_Insert(C_ARRAY void* tag_array_p,const void *in_data,void *in_cur)
{
	ARRAY_HEAD *array_head;
	void *array_op;
	void *tmp_space;
	C_ARRAY void* tag_array;

	tag_array=*(void**)tag_array_p;
	array_head=array_Head(tag_array);
	array_op=(void*)tag_array;
	
	tmp_space=(void*)malloc(array_head->data_size);
		if(tmp_space==NULL)return NULL;
	memcpy(tmp_space,&((char*)array_op)[(array_head->array_length-1)*(array_head->data_size)],array_head->data_size);
	
	memmove(((void*)in_cur)+array_head->data_size,(void*)in_cur,(array_op+(array_head->array_length-1)*(array_head->data_size))-((void*)in_cur));
	memcpy(in_cur,in_data,array_head->data_size);
	array_op=(C_ARRAY void*)array_Append(tmp_space,(C_ARRAY void*)array_op);
	free(tmp_space);
	*(void**)tag_array_p=(C_ARRAY void*)array_op;

	return (C_ARRAY void*)array_op;
	
};

C_ARRAY void* array_Remove(C_ARRAY const void* tag_array_p,void *in_cur)
{
	ARRAY_HEAD *array_head;
	void *array_op;
	C_ARRAY void* tag_array;

	tag_array=*(void**)tag_array_p;
	array_head=array_Head(tag_array);
	array_op=(char*)tag_array;
	
	if(array_head->free_fun!=NULL)array_head->free_fun(in_cur);
	memmove(in_cur,(void*)in_cur+array_head->data_size,(array_op+(array_head->array_length-1)*(array_head->data_size))-((void*)in_cur));
	--array_head->array_length;
	*(void**)tag_array_p=(C_ARRAY void*)array_op;
	
	return (C_ARRAY void*)array_op;
};

void array_Drop(C_ARRAY void* tag_array_p)
{
	ARRAY_HEAD *array_head;
	char *array_op;
	C_ARRAY void* tag_array;

	tag_array=*(void**)tag_array_p;
	array_head=array_Head(tag_array);
	if(array_head->free_fun!=NULL&&array_head->array_length>0)for(array_op=tag_array;(array_op-(char*)tag_array)<(array_Length(tag_array)*(array_head->data_size));array_op+=(array_Head(tag_array)->data_size))array_head->free_fun(array_op);

	free(array_head);
};
