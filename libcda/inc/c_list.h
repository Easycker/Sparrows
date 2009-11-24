#ifndef C_LIST_H
#define C_LIST_H

#include "c_mem.h"
#include "c_define.h"

typedef struct list_head
{
	void *next;
}LIST_HEAD;

typedef struct list_type
{
	size_t data_size;
	void *list_head;
	void *list_tail;
	C_POOL data_pool;
	size_t list_length;
	void (*free_fun)(void* tag_data);
}C_LIST;

#define list_Head(in_cur) ((LIST_HEAD*)in_cur-1)

#define list_First(chain) (((C_LIST*)chain)->list_head)

#define list_Tail(chain) (((C_LIST*)chain)->list_tail)

#define list_Create(tag_chain,data_size) (list_Create_Ex(tag_chain,data_size,NULL))

C_LIST* list_Create_Ex(C_LIST *tag_chain,size_t data_size,void (*free_fun)(void *tag_data));

void* list_Append(const void *in_data,C_LIST *tag_chain);

void* list_Insert_Ex(const void *in_data,void *in_cur,C_LIST *tag_chain);

void* list_Insert_Head(const void *in_data,C_LIST *tag_chain);

#define list_Next(in_cur) (list_Head(in_cur)->next)

void list_Remove_Ex(void *in_cur,C_LIST *tag_chain);

void list_Drop(C_LIST *tag_chain);

/*here is dlist's place*/
typedef struct dlist_head
{
	void *prev;
	void *next;
}DLIST_HEAD;

typedef struct dlist_type
{
	size_t data_size;
	void *dlist_head;
	void *dlist_tail;
	C_POOL data_pool;
	size_t dlist_length;
	void (*free_fun)(void* tag_data);
}C_DLIST;

#define dlist_Head(in_cur) ((DLIST_HEAD*)in_cur-1)

#define dlist_First(dlist) (((C_DLIST*)dlist)->dlist_head)

#define dlist_Tail(dlist) (((C_DLIST*)dlist)->dlist_tail)

#define dlist_Create(tag_dlist,data_size) (dlist_Create_Ex(tag_dlist,data_size,NULL))

C_DLIST* dlist_Create_Ex(C_DLIST *tag_dlist,size_t data_size,void (*free_fun)(void *tag_data));

void* dlist_Append(const void *in_data,C_DLIST *tag_dlist);

void* dlist_Insert(const void *in_data,void *in_cur,C_DLIST *tag_dlist);

#define dlist_Prev(in_cur) (dlist_Head(in_cur)->prev)
#define dlist_Next(in_cur) (dlist_Head(in_cur)->next)

void dlist_Remove(void *in_cur,C_DLIST *tag_dlist);

void dlist_Drop(C_DLIST *tag_dlist);

#endif
