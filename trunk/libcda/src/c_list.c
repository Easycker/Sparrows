#include "c_list.h"

C_LIST* list_Create_Ex(C_LIST *tag_chain,size_t data_size,void (*free_fun)(void *tag_data))
{
	if(tag_chain==NULL)return NULL;
	
	tag_chain->list_head=NULL;
	if(pool_Create(&tag_chain->data_pool,data_size+sizeof(LIST_HEAD))==NULL)return NULL;
	
	tag_chain->list_length=0;
	tag_chain->data_size=data_size;
	tag_chain->free_fun=free_fun;
	
	return tag_chain;
};

void* list_Append(const void *in_data,C_LIST *tag_chain)
{
	LIST_HEAD *new_op;
	
	new_op=pool_Malloc(&tag_chain->data_pool);
	new_op->next=NULL;
	++new_op;
	MEMCPY_(new_op,in_data,tag_chain->data_size);
	if(tag_chain->list_head!=NULL)
	{
		(list_Head(tag_chain->list_tail))->next=new_op;
		tag_chain->list_tail=new_op;
	}
	else
	{
		tag_chain->list_head=new_op;
		tag_chain->list_tail=new_op;
	};
	++tag_chain->list_length;
	return (void*)new_op;
};

void* list_Insert_Ex(const void *in_data,void *in_cur,C_LIST *tag_chain)
{
	LIST_HEAD *list_head;
	LIST_HEAD *cur_head;
	LIST_HEAD *new_op;
	
	assert(in_data!=NULL&&in_cur!=NULL&&tag_chain!=NULL);
	if(in_data!=NULL&&in_cur!=NULL&&tag_chain!=NULL)
	{
		list_head=list_Head(in_cur);
		if(list_head->next!=NULL)
		{
			new_op=pool_Malloc(&tag_chain->data_pool);
			cur_head=list_Head(in_cur);
			new_op->next=cur_head->next;
			++new_op;
			MEMCPY_(new_op,in_data,tag_chain->data_size);
			cur_head->next=(void*)new_op;
			++(tag_chain->list_length);
		}
		else
		{
			list_Append(in_data,tag_chain);
		};
		return (void*)new_op;
	}
	return NULL;
};

void* list_Insert_Head(const void *in_data,C_LIST *tag_chain)
{
	LIST_HEAD *new_op;
	
	assert(in_data!=NULL&&tag_chain!=NULL);
	if(in_data!=NULL&&tag_chain!=NULL)
	{
		new_op=(LIST_HEAD*)pool_Malloc(&tag_chain->data_pool);
		++new_op;
		MEMCPY_(new_op,in_data,tag_chain->data_size);
		if(tag_chain->list_head!=NULL)
		{
			list_Head(new_op)->next=list_Head(tag_chain->list_head)->next;
			tag_chain->list_head=new_op;
		}
		else
		{
			list_Head(new_op)->next=NULL;
			tag_chain->list_head=new_op;
		};
		++(tag_chain->list_length);
		return new_op;
	}
	return NULL;
};

void list_Remove_Ex(void *in_cur,C_LIST *tag_chain)
{
	LIST_HEAD *cur_head;
	LIST_HEAD *tmp_ptr;
	
	assert(in_cur!=NULL&&tag_chain!=NULL);
	if(in_cur!=NULL&&tag_chain!=NULL)
	{
		cur_head=list_Head(in_cur);
		tmp_ptr=cur_head->next;
		if(tmp_ptr!=NULL)
		{
			if(tag_chain->free_fun!=NULL)tag_chain->free_fun(*(void**)tmp_ptr);
			cur_head->next=(list_Head(cur_head->next))->next;
			--tag_chain->list_length;
			list_Next(tmp_ptr)=NULL;
			pool_Free((tmp_ptr-1),&tag_chain->data_pool);
		};
	}
};

void list_Drop(C_LIST *tag_chain)
{
	void *tmp_ptr;
	
	if(tag_chain->free_fun!=NULL&&tag_chain->list_length>0)for(tmp_ptr=list_First(tag_chain);tmp_ptr!=NULL;tmp_ptr=list_Next(tmp_ptr))tag_chain->free_fun(*(void**)tmp_ptr);
	pool_Drop(&tag_chain->data_pool);
};

/*here is dlist's place*/
C_DLIST* dlist_Create_Ex(C_DLIST *tag_dlist,size_t data_size,void (*free_fun)(void *tag_data))
{
	if(tag_dlist==NULL)return NULL;
	
	tag_dlist->dlist_head=NULL;
	if(pool_Create(&tag_dlist->data_pool,data_size+sizeof(C_DLIST))==NULL)return NULL;
	
	tag_dlist->dlist_length=0;
	tag_dlist->data_size=data_size;
	tag_dlist->free_fun=free_fun;
	
	return tag_dlist;
};

void* dlist_Append(const void *in_data,C_DLIST *tag_dlist)
{
	DLIST_HEAD *new_op;
	
	assert(in_data!=NULL&&tag_dlist!=NULL);
	if(in_data!=NULL&&tag_dlist!=NULL)
	{
		new_op=pool_Malloc(&tag_dlist->data_pool);
		++new_op;
		dlist_Next(new_op)=NULL;
		MEMCPY_(new_op,in_data,tag_dlist->data_size);
		if(tag_dlist->dlist_head!=NULL)
		{
			/*dlist_Head(new_op)->prev=tag_dlist->dlist_tail;*/
			dlist_Prev(new_op)=tag_dlist->dlist_tail;
			(dlist_Head(tag_dlist->dlist_tail))->next=new_op;
			tag_dlist->dlist_tail=new_op;
		}
		else
		{
			dlist_Prev(new_op)=NULL;
			tag_dlist->dlist_head=new_op;
			tag_dlist->dlist_tail=new_op;
		};
		++tag_dlist->dlist_length;
		return (void*)new_op;
	}
	return NULL;
};

void* dlist_Insert(const void *in_data,void *in_cur,C_DLIST *tag_dlist)
{
	DLIST_HEAD *head;

	assert(in_data!=NULL&&in_cur!=NULL&&tag_dlist!=NULL);
	if(in_data!=NULL&&in_cur!=NULL&&tag_dlist!=NULL)
	{
		head=pool_Malloc(&tag_dlist->data_pool);
		++head;
		MEMCPY_(head,in_data,tag_dlist->data_size);
		if(dlist_Prev(in_cur)!=NULL)
		{
			dlist_Next(dlist_Prev(in_cur))=head;
			dlist_Prev(head)=dlist_Prev(in_cur);
			dlist_Next(head)=in_cur;
			dlist_Prev(in_cur)=head;
		}
		else
		{
			dlist_Next(head)=in_cur;
			dlist_Prev(head)=NULL;
			dlist_Prev(in_cur)=head;
			tag_dlist->dlist_head=head;
		};
		++tag_dlist->dlist_length;
		return head;
	}
	return NULL;
};

void dlist_Remove(void *in_cur,C_DLIST *tag_dlist)
{
	DLIST_HEAD *head;

	assert(in_cur!=NULL&&tag_dlist!=NULL);
	if(in_cur!=NULL&&tag_dlist!=NULL)
	{
		if(dlist_Prev(in_cur)!=NULL)
		{
			dlist_Next(dlist_Prev(in_cur))=dlist_Next(in_cur);
			if(dlist_Next(in_cur)==NULL)tag_dlist->dlist_tail=dlist_Prev(in_cur);
			else
			{
				dlist_Prev(dlist_Next(in_cur))=dlist_Prev(in_cur);
			};
		}
		else
		{
			tag_dlist->dlist_head=dlist_Next(in_cur);
			if(dlist_Next(in_cur)!=NULL)dlist_Prev(dlist_Next(in_cur))=NULL;
			else
			{
				tag_dlist->dlist_head=NULL;
				tag_dlist->dlist_tail=NULL;
			};
			if(dlist_Next(in_cur)==NULL)tag_dlist->dlist_tail=dlist_Next(in_cur);
		};
		head=in_cur;
		dlist_Next(head)=NULL;
		dlist_Prev(head)=NULL;
		--head;
		pool_Free(head,&tag_dlist->data_pool);
		--tag_dlist->dlist_length;
	}
};

void dlist_Drop(C_DLIST *tag_dlist)
{
	void *tmp_ptr;
	
	if(tag_dlist->free_fun!=NULL&&tag_dlist->dlist_length>0)for(tmp_ptr=dlist_First(tag_dlist);tmp_ptr!=NULL;tmp_ptr=dlist_Next(tmp_ptr))tag_dlist->free_fun(*(void**)tmp_ptr);
	pool_Drop(&tag_dlist->data_pool);
};
