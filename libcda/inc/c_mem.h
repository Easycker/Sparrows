#ifndef C_MEM_H
#define C_MEM_H

#include "c_define.h"

#define __MEM_POOL_SIZE 64

typedef struct block_node
{
	struct block_node *next_block;
}C_POOL_BLOCK;

typedef struct pool_node
{
	struct block_node *pool_mem;
	struct pool_node *pool_next;
}C_POOL_NODE;

typedef struct c_pool
{
	size_t data_size;
	struct block_node *free_mem;
	struct pool_node *head_node;
	struct pool_node *last_node;
	int flag;
}C_POOL;

C_POOL* pool_Create(C_POOL *tag_pool,size_t data_size);

	C_POOL_NODE* node_Create(size_t data_size);

/*void* MEMCPY_(void *dest,const void *src,size_t n);*/
	
C_POOL* pool_Create_Ex(C_POOL *tag_pool,size_t data_size,void *free_mem,size_t mem_size);

C_POOL* node_Append(C_POOL *tag_pool,void *free_mem,size_t mem_size);

void* pool_Malloc(struct c_pool *tag_pool);

void pool_Free(void *free_mem,struct c_pool *tag_pool);

void* pool_Drop(struct c_pool *tag_pool);

#endif
