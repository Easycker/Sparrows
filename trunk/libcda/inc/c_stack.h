#ifndef C_STACK_H
#define C_STACK_H

#include "c_array.h"

#define C_STACK

#define stack_Head array_Head

#define queue_Head stack_Head

#define stack_Create(data_size) (array_Create(data_size))

#define queue_Create stack_Create

#define stack_Create_Ex(data_size,free_fun) (array_Create_Ex(data_size,free_fun))

#define queue_Create_Ex stack_Create_Ex

#define stack_Append(tag_array,in_data) (array_Append(tag_array,in_data))

#define queue_Append stack_Append

#define stack_Get(tag_array) ((&tag_array[(array_Head(tag_array))->array_length-1]))

#define queue_Get(tag_array) ((tag_array[0]))

#define stack_Remove(tag_array) (array_Remove(tag_array,tag_array[array_Head(*tag_array)->array_length-1]))

#define queue_Remove(tag_array) (array_Remove(tag_array,tag_array[0]))

#define stack_Drop(tag_array) (array_Drop(tag_array))

#define queue_Drop stack_Drop

#endif
