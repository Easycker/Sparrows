#ifndef C_HASH_H
#define C_HASH_H

#include "c_define.h"
#include "c_mem.h"

typedef void* HASH_DATA;

typedef struct hash_node
{
	HASH_DATA next;
}HASH_HEAD;

typedef struct hash_type
{
	UINT_ (*hash_fun)();/*UINT_ (*hash_fun)(void *in_data);*/
	BOOL_ (*ensure_fun)();/*BOOL_ (*ensure_fun)(void *lhs,void *rhs);*/
	size_t data_size;
	UINT_ max_size;
	HASH_HEAD *peer_pool;
	C_POOL data_pool;
	void (*free_fun)();/*void (*free_fun)(void* tag_data);*/
}C_HASH;

#define hash_next(tag_peer) (((HASH_HEAD*)tag_peer-1)->next)

#define hash_Create(tag_hash,hash_fun,ensure_fun,data_size,max_size) (hash_Create_Ex(tag_hash,hash_fun,ensure_fun,data_size,max_size,NULL))

C_HASH* hash_Create_Ex(C_HASH *tag_hash,UINT_ (*hash_fun)(),BOOL_ (*ensure_fun)(),size_t data_size,UINT_ max_size,void (*free_fun)());

HASH_DATA hash_Append(C_HASH *tag_hash,HASH_DATA in_data);

HASH_DATA hash_Get(C_HASH *tag_hash,HASH_DATA in_data);

void hash_Remove(C_HASH *tag_hash,HASH_DATA in_data);

void hash_Drop(C_HASH *tag_hash);

#endif
