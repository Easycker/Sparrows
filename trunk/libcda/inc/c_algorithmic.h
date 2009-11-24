#ifndef C_ALGORITHMIC_H
#define C_ALGORITHMIC_H

#include "c_array.h"
#include "c_list.h"
#include <stdint.h>

#define ptr2index(array,ptr) (((intptr_t)ptr)>=((intptr_t)array)?(((intptr_t)ptr-(intptr_t)array)/(array_Head(array)->data_size)):0)

#define heap_Parent(array,ptr) (((ptr2index(array,ptr)+1)/2-1)>0?(&(((char*)array)[(array_Head(array))->data_size*((ptr2index(array,ptr)+1)/2-1)])):NULL)

#define heap_Left(array,ptr) (((ptr2index(array,ptr)+1)*2-1)<array_Head(array)->array_length?(&(((char*)array)[(array_Head(array))->data_size*((ptr2index(array,ptr)+1)*2-1)])):NULL)

#define heap_Right(array,ptr) (((ptr2index(array,ptr)+1)*2)<array_Head(array)->array_length?(&(((char*)array)[(array_Head(array))->data_size*((ptr2index(array,ptr)+1)*2)])):NULL)

C_ARRAY void* array_Maxheapify(C_ARRAY void *array,void *ptr,BOOL_ (*compare)(void *lhs,void *rhs));

C_ARRAY void* array_Sort(C_ARRAY void *array,BOOL_ (*compare)(void *lhs,void *rhs));

C_LIST* list_Sort(C_LIST *list,BOOL_ (*compare)(void *lhs,void *rhs));

C_DLIST* dlist_Sort(C_LIST *list,BOOL_ (*compare)(void *lhs,void *rhs));

#endif
