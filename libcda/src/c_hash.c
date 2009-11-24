#include "c_hash.h"

C_HASH* hash_Create_Ex(C_HASH *tag_hash,UINT_ (*hash_fun)(),BOOL_ (*ensure_fun)(),size_t data_size,UINT_ max_size,void (*free_fun)())
{
	tag_hash->hash_fun=hash_fun;
	tag_hash->ensure_fun=ensure_fun;
	tag_hash->data_size=data_size;
	tag_hash->peer_pool=NULL;
	tag_hash->max_size=max_size;
	tag_hash->peer_pool=malloc(max_size*sizeof(HASH_HEAD));
	tag_hash->free_fun=free_fun;
	if(tag_hash->peer_pool==NULL)return NULL;
	memset(tag_hash->peer_pool,0,max_size*sizeof(HASH_HEAD));

	if(!pool_Create(&tag_hash->data_pool,sizeof(HASH_HEAD)+data_size))
	{
		free(tag_hash->peer_pool);
		return NULL;
	};
	return tag_hash;
};

HASH_DATA hash_Append(C_HASH *tag_hash,HASH_DATA in_data)
{
	UINT_ data_code;
	HASH_HEAD *op;
	HASH_HEAD *op_op;

	data_code=0;
	data_code=tag_hash->hash_fun(in_data);
	op=(HASH_HEAD*)pool_Malloc(&tag_hash->data_pool);
	op->next=NULL;
	++op;
	memcpy(op,in_data,tag_hash->data_size);
	op_op=&(tag_hash->peer_pool[data_code]);
	if(op_op->next==NULL)
	{
		op_op->next=op;
	}
	else
	{
		for(op_op=op_op->next;hash_next(op_op)!=NULL;op_op=hash_next(op_op));
		hash_next(op_op)=op;
	};
	return op;	
};

HASH_DATA hash_Get(C_HASH *tag_hash,HASH_DATA in_data)
{
	UINT_ data_code;
	HASH_HEAD *op;

	data_code=tag_hash->hash_fun(in_data);
	op=&(tag_hash->peer_pool[data_code]);
	
	if(op->next!=NULL)
	{
		for(op=op->next;;op=(HASH_HEAD*)hash_next(op))
		{
			if(tag_hash->ensure_fun(in_data,op))
			{
				return (HASH_DATA)op;
			};
			if(hash_next(op)==NULL)break;
		};
	};
	return NULL;
};

void hash_Remove(C_HASH *tag_hash,HASH_DATA in_data)
{
	UINT_ data_code;
	HASH_HEAD *op;
	HASH_HEAD *tmp_op;
	HASH_HEAD *op_prev;

	data_code=tag_hash->hash_fun(in_data);
	op=&tag_hash->peer_pool[data_code];
	if(op->next!=NULL)
	{
		if(hash_next(op->next)==NULL)
		{
			if(tag_hash->free_fun!=NULL)tag_hash->free_fun(op->next);
			pool_Free(((HASH_HEAD*)(op->next)-1),&tag_hash->data_pool);
			op->next=NULL;
		}
		else
		{
			op_prev=NULL;
			for(tmp_op=(HASH_HEAD*)op->next;op!=NULL;)
			{
				if(tag_hash->ensure_fun(in_data,tmp_op))
				{
					if(op_prev==NULL)
					{
						op->next=hash_next(tmp_op);
					}
					else
					{
						hash_next(op_prev)=hash_next(tmp_op);
					};
					if(tag_hash->free_fun!=NULL)tag_hash->free_fun(tmp_op);
					pool_Free((tmp_op-1),&tag_hash->data_pool);
				};
				op_prev=tmp_op;
				tmp_op=(HASH_HEAD*)hash_next(tmp_op);
			};
		};
	};
};

void hash_Drop(C_HASH *tag_hash)
{
	size_t i;
	HASH_HEAD *op;

	if(tag_hash->free_fun!=NULL)
	{
		for(i=0;i<tag_hash->max_size;++i)
		{
			if((tag_hash->peer_pool[i]).next!=NULL)
			{
				for(op=tag_hash->peer_pool[i].next;hash_next(op)!=NULL;op=hash_next(op))tag_hash->free_fun(op);
			};
		};
	};
	free(tag_hash->peer_pool);
	pool_Drop(&tag_hash->data_pool);
};
