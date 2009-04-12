#include "cgi_funs.h"

BOOL_ session_Compare(SESSION_NODE *lhs,SESSION_NODE *rhs)
{
	return STRCMP_(lhs->user_data.user_id,rhs->user_data.user_id)>0;
	/*return (lhs->session_id)>(rhs->session_id);*/
};

BOOL_ session_Eq(struct session_node *lhs,struct session_node *rhs)
{
	return STRCMP_(lhs->user_data.user_id,rhs->user_data.user_id)==0;
	/*return lhs->session_id==rhs->session_id;*/
};

BOOL_ session_Min(SESSION_NODE *lhs,SESSION_NODE *rhs)
{
	return TRUE_;
};

BOOL_ session_Max(SESSION_NODE *lhs,SESSION_NODE *rhs)
{
	return FALSE_;
};

USER_DATA* user_Create(USER_DATA *data)
{
	data->user_name=string_Create();
	data->user_id=string_Create();
};

struct session_list* session_Create(SESSION_LIST *tag_list,size_t data_size)
{
	BOOL_ (*com_fun)();
	
	tag_list->data_size=data_size;
	tag_list->last_id=0;
	com_fun=&session_Compare;
	
	prtree_Create(&tag_list->session_tree,sizeof(struct session_node),com_fun);
	pool_Create(&tag_list->data_pool,data_size);
	
	return tag_list;
};

UINT_ session_Append(USER_DATA *user_data,struct session_list *in_list)
{
	SESSION_NODE fake_node;
	SESSION_NODE *node_op;
	BOOL_ (*match_fun)();
	
	match_fun=&session_Eq;
	/*memcpy(&fake_node.user_data.user_name,user_data->user_name,NAME_SIZE_);*/
	
	STRCPY_(fake_node.user_data.user_name,user_data->user_name);

	node_op=(SESSION_NODE*)prtree_Get(&fake_node,match_fun,&in_list->session_tree);
	if(node_op!=NULL)prtree_Remove(node_op,&in_list->session_tree);
	node_op=(SESSION_NODE*)prtree_Append(&fake_node,&in_list->session_tree);
	
	node_op->last_time=time(NULL);
	
	node_op->session_id=in_list->last_id;
	in_list->last_id++;/*not safe*/
	memcpy(&node_op->user_data,user_data,sizeof(USER_DATA));
	
	return (in_list->last_id-1);
};

struct session_node* session_Get(CHAR_ *user_name,struct session_list *in_list)
{
	SESSION_NODE fake_node;
	SESSION_NODE *tag_node;
	BOOL_ (*match_fun)();
	
	match_fun=&session_Eq;
	
	/*memcpy(&fake_node.user_data.user_name,user_name,NAME_SIZE_);*/
	STRCPY_(fake_node.user_data.user_id,user_name);
	
	tag_node=prtree_Get(&fake_node,match_fun,&in_list->session_tree);
		
	return tag_node;	
};

void session_Remove(USER_DATA *user_data,SESSION_LIST *in_list)
{
	SESSION_NODE fake_node;
	SESSION_NODE *node_op;
	BOOL_ (*match_fun)();
	
	match_fun=&session_Eq;
	/*memcpy(&fake_node.user_data.user_name,user_data->user_name,NAME_SIZE_);*/
	STRCPY_(fake_node.user_data.user_name,user_data->user_name);
	
	node_op=(SESSION_NODE*)prtree_Get(&fake_node,match_fun,&in_list->session_tree);
	
	prtree_Remove(node_op,&in_list->session_tree);
};

void session_Clean(struct session_list *in_list)
{
	
};

void session_Drop(struct session_list *in_list)
{
	prtree_Drop(&in_list->session_tree);
	pool_Drop(&in_list->data_pool);

	in_list=NULL;
};

CHAR_* ca_Get(CHAR_ *tag_array,CHAR_ *key_string,CHAR_ *tag_string)
{
	CHAR_ *get_res;
	KMP_KEY kmp_pre;
	CHAR_* char_op;
	CHAR_ end_char;
	
	string_Kprepare(&kmp_pre,key_string);
	char_op=string_Knsearch(tag_string,&kmp_pre,STRLEN_(tag_string));

	get_res=array_Create(sizeof(CHAR_));

	for(char_op+=(STRLEN_(key_string)+1);!((*char_op)==ENCODE_('\0')||(*char_op)==ENCODE('&')||(*char_op)==ENCODE_(';'));++char_op)
	{
		get_res=array_Append(get_res,char_op);
	};
				
	end_char=ENCODE_('\0');
	get_res=array_Append(get_res,&end_char);
		
	memcpy(tag_array,get_res,(array_Head(get_res)->array_length)*sizeof(CHAR_));
	
	string_Kfree(&kmp_pre);
	
	return tag_array;
	
};

CHAR_* get_Post(CHAR_ *tag_string)
{
	size_t post_length;
	CHAR_ *char_op;
	
	if(getenv("CONTENT_LENGTH")!=NULL)
	{
		post_length=atoi(getenv("CONTENT_LENGTH"));

		if(post_length>0&&post_length<128)
		{
			char_op=tag_string;
			for(*char_op=GETC_(stdin);*char_op!=EOF;*(++char_op)=GETC_(stdin));
			*char_op=ENCODE_('\0');
			
			return tag_string;
		};
	};

	return NULL;
};

BOOL_ debug_Output(CHAR_ *message)
{
	/*
	FILE *fp;
	
	fp=fopen("/tmp/debug.out","a");
		
	fputs(message,fp);
	
	fclose(fp);
	
	return TRUE_;
	*/
};
