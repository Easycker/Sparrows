#include "request.h"

BOOL_ arg_Comp(void *lhs,void *rhs)
{
	return (strcmp(((ARG_NODE*)lhs)->name,((ARG_NODE*)rhs)->name)>0);
}

BOOL_ arg_Equal(void *lhs,void *rhs)
{
	return !(strcmp(((ARG_NODE*)lhs)->name,((ARG_NODE*)rhs)->name));
}

void arg_Free(void *arg)
{
	string_Drop(&((ARG_NODE*)arg)->name);
}

HTTP_REQUEST* request_Create(HTTP_REQUEST *request)
{
	request->recv_data=NULL;
	request->path=NULL;
	request->host=NULL;
	request->accept=NULL;
	request->accept_charset=NULL;
	request->accept_encoding=NULL;
	request->accept_language=NULL;
	request->cookies=NULL;
	request->referer=NULL;
	request->user_agent=NULL;
	request->query_string=NULL;
	request->remote_addr=NULL;
	request->content_type=NULL;
	request->content_length=NULL;
	ERROR_OUT_(stderr,"malloc a request\n");

	return request;
};

HTTP_REQUEST* request_Head(C_BHTREE *tree,HTTP_REQUEST *request,CHAR_* const string,size_t length)
{
	ARG_NODE fake_node;
	ARG_NODE *node;

	char *ptr;
	char arg1[ARG_LEN_];
	char arg2[ARG_LEN_];
	char arg3[ARG_LEN_];
	char arg4[ARG_LEN_];
	char arg5[ARG_LEN_];

	char first_line[ARG_LEN_];
	char per_line[ARG_LEN_];
	int flag=0;

	char *str_ptr;
	char *prefix_ptr;

	ERROR_OUT_(stderr,"REQUEST STRING IS:\n%s",string);

	sprintf(first_line,"%%%ds %%%ds %%%ds",ARG_LEN_,ARG_LEN_,ARG_LEN_);
	sprintf(per_line,"%%%ds %%%ds",ARG_LEN_,ARG_LEN_);

	/*THE HTTP HEADER*/
	ptr=string;
	if(sscanf(ptr,first_line,arg1,arg2,arg3)!=3)ERROR_EXIT_;
	request->type=(*arg1=='P'?POST:GET);
	ERROR_OUT_(stderr,"arg2 is %s\n",arg2);
	/*PROCESS THE GET*/
	for(ptr=arg2;*ptr!=' '&&*ptr!='?'&&*ptr!='\0';++ptr);
	if(*ptr=='?')
	{
		*ptr=' ';
		ptr=arg2;
		if(sscanf(ptr,per_line,arg4,arg5)!=2)ERROR_EXIT_;
		request->path=string_Create_Ex(arg4);
		request->query_string=string_Create_Ex(arg5);
	}
	else
	{
		request->path=string_Create_Ex(arg2);
	}

	for(ptr=string;flag<2&&*ptr!='\0';++ptr)
	{
		if(*ptr=='\n')++flag;
		if(*ptr==' ')*ptr='#';
		if(*ptr==':'&&flag>0)
		{
			*ptr='\t';
			--flag;
		}
	}

	fake_node.name=string_Create();
	ptr=string;
	for(;*ptr!='\n'&&*ptr!='\0';++ptr);
	++ptr;
	request->alive=FALSE_;
	while(*ptr!='\r'&&*ptr!='\n'&&*ptr!='\0')
	{
		sscanf(ptr,per_line,arg1,arg2);
		string_Set(&fake_node.name,arg1);
		ERROR_OUT_(stderr,"fake node is %s\n",fake_node.name);
		node=bhtree_Get(&fake_node,&arg_Equal,tree);

		if(node!=NULL)
		{
			for(prefix_ptr=arg2;*prefix_ptr!='\0';++prefix_ptr)if(*prefix_ptr=='#')*prefix_ptr=' ';
			if(node->type==ARG_BOOL)
			{
				ERROR_OUT_(stderr,"ok add a bool:%s\n",arg2);
				str_ptr=(char*)request;
				str_ptr+=node->offset;
				for(prefix_ptr=arg2;*prefix_ptr==' '&&*prefix_ptr!='\0';++prefix_ptr);
				if(*prefix_ptr=='K'||*prefix_ptr=='k')
				{
					*(BOOL_*)str_ptr=TRUE_;
				}
				else *(BOOL_*)str_ptr=FALSE_;
			}
			else
			{
				ERROR_OUT_(stderr,"ok add a str:%s\n",arg2);
				str_ptr=(char*)request;
				str_ptr+=node->offset;
				if((*(C_STRING*)str_ptr)==NULL)*(C_STRING*)str_ptr=string_Create_Ex(arg2);
			}
		}

		for(;*ptr!='\n'&&*ptr!='\0';++ptr);
		if(*ptr=='\0')break;
		++ptr;
	}
	string_Drop(&fake_node.name);
	
	return request;

	fail_return:
	return NULL;
};

void request_Free(HTTP_REQUEST *request,HEAD_SHARE *head_share)
{
	if(request->recv_data!=NULL)pool_Free(request->recv_data,&(head_share->head_pool));

	if(request->path!=NULL)string_Drop(&request->path);
	if(request->host!=NULL)string_Drop(&request->host);
	if(request->accept!=NULL)string_Drop(&request->accept);
	if(request->accept_charset!=NULL)string_Drop(&request->accept_charset);
	if(request->accept_encoding!=NULL)string_Drop(&request->accept_encoding);
	if(request->accept_language!=NULL)string_Drop(&request->accept_language);
	if(request->cookies!=NULL)string_Drop(&request->cookies);
	if(request->referer!=NULL)string_Drop(&request->referer);
	if(request->user_agent!=NULL)string_Drop(&request->user_agent);
	if(request->query_string!=NULL)string_Drop(&request->query_string);
	if(request->remote_addr!=NULL)string_Drop(&request->remote_addr);
	if(request->content_type!=NULL)string_Drop(&request->content_type);
	if(request->content_length!=NULL)string_Drop(&request->content_length);

	ERROR_OUT_(stderr,"leave a request\n");
};

UINT_ head_Tinyhash(HEAD_FD *head)
{
	return head->fd&HASH_SPACE_;
};

BOOL_ head_Ensure(HEAD_FD *lhs,HEAD_FD *rhs)
{
	return lhs->fd==rhs->fd;
};

void head_Free(HEAD_FD *fd)
{
	/*if(fd->buf!=NULL)pool_Free(*/
	/*close(fd->fd);*/
};

HEAD_SHARE* head_Init(size_t buf_size,UINT_ max_head,C_ARRAY HOST_TYPE *host_list)
{
	HEAD_SHARE *share;
	ARG_NODE arg;

	share=(HEAD_SHARE*)malloc(sizeof(HEAD_SHARE));
	if(share==NULL)
	{
		ERROR_OUT_(stderr,ENCODE_("SHARE IS NULL\n"));
		return NULL;
	};
	share->max_head=max_head;
	hash_Create_Ex(&share->fd_list,&head_Tinyhash,&head_Ensure,sizeof(HEAD_FD),(UINT_)HASH_SPACE_,&head_Free);

	/*initial args*/
	if(bhtree_Create_Ex(&share->arg_tree,sizeof(ARG_NODE),&arg_Comp,&arg_Free)==NULL)ERROR_EXIT_;

	arg.name=string_Create_Ex("Host");
	arg.type=ARG_STR;
	arg.offset=(intptr_t)&((struct http_request*)0)->host;
	bhtree_Append(&arg,&share->arg_tree);

	arg.name=string_Create_Ex("Connection");
	arg.type=ARG_BOOL;
	arg.offset=(intptr_t)&((struct http_request*)0)->alive;
	bhtree_Append(&arg,&share->arg_tree);

	arg.name=string_Create_Ex("Content-Type");
	arg.type=ARG_STR;
	arg.offset=(intptr_t)&((struct http_request*)0)->content_type;
	bhtree_Append(&arg,&share->arg_tree);

	arg.name=string_Create_Ex("Content-Length");
	arg.type=ARG_STR;
	arg.offset=(intptr_t)&((struct http_request*)0)->content_length;
	bhtree_Append(&arg,&share->arg_tree);

	arg.name=string_Create_Ex("Accept");
	arg.type=ARG_STR;
	arg.offset=(intptr_t)&((struct http_request*)0)->accept;
	bhtree_Append(&arg,&share->arg_tree);

	arg.name=string_Create_Ex("Accept-Charset");
	arg.type=ARG_STR;
	arg.offset=(intptr_t)&((struct http_request*)0)->accept_charset;
	bhtree_Append(&arg,&share->arg_tree);

	arg.name=string_Create_Ex("Accept-Encoding");
	arg.type=ARG_STR;
	arg.offset=(intptr_t)&((struct http_request*)0)->accept_encoding;
	bhtree_Append(&arg,&share->arg_tree);

	arg.name=string_Create_Ex("Accept-Language");
	arg.type=ARG_STR;
	arg.offset=(intptr_t)&((struct http_request*)0)->accept_language;
	bhtree_Append(&arg,&share->arg_tree);

	arg.name=string_Create_Ex("Cookies");
	arg.type=ARG_STR;
	arg.offset=(intptr_t)&((struct http_request*)0)->cookies;
	bhtree_Append(&arg,&share->arg_tree);

	arg.name=string_Create_Ex("Referer");
	arg.type=ARG_STR;
	arg.offset=(intptr_t)&((struct http_request*)0)->referer;
	bhtree_Append(&arg,&share->arg_tree);

	arg.name=string_Create_Ex("User-Agent");
	arg.type=ARG_STR;
	arg.offset=(intptr_t)&((struct http_request*)0)->user_agent;
	bhtree_Append(&arg,&share->arg_tree);

	pool_Create(&(share->head_pool),share->max_head);
	share->buf_size=buf_size;
	share->host_list=host_list;
	return share;

fail_return:
	return NULL;
};

int head_Work(HEAD_SHARE *share,HTTP_CONNECT *connect)
{
	HEAD_FD *head;
	HEAD_FD fake_head;
	HTTP_REQUEST request;
	regmatch_t preg_match;
	int state;
	int return_state;
	int len;
	int i;
	int j;

	fake_head.fd=connect->fd;
	if(&share->fd_list==NULL)ERROR_OUT_(stderr,ENCODE_("null\n"));
	head=hash_Get(&share->fd_list,&fake_head);
	if(head==NULL)
	{
		ERROR_OUT_(stderr,ENCODE_("HERE CREATE A NEW HEAD\n"));
		fake_head.fd=connect->fd;
		fake_head.buf_len=0;
		fake_head.buf=pool_Malloc(&(share->head_pool));
		hash_Append(&share->fd_list,&fake_head);
		head=hash_Get(&share->fd_list,&fake_head);
	};
	while(1)
	{
		len=recv(head->fd,&head->buf[head->buf_len],share->buf_size,0);
		ERROR_OUT_(stderr,"len is %d\n",len);
		if(len<0)
		{
			if(errno==EAGAIN)
			{
				/*head->buf_len+=share->buf_size;*/
				return WORK_GOON_;
			}
			else
			{
				return WORK_CLOSE_;
			};
		}
		else if(len==0||len<share->buf_size||head->buf_len+share->buf_size>=share->max_head)
		/*else if(len<share->buf_size||head->buf_len+share->buf_size>=share->max_head)*/
		{
			ERROR_OUT_(stderr,ENCODE_("HEAD RECV DONE,THE FD IS;%d\n"),connect->fd);
			/*all data had been recv*/
			head->buf_len+=len;
			head->buf[head->buf_len]='\0';
			if(request_Create(&request)==NULL)goto fail_return;
			if(request_Head(&share->arg_tree,&request,head->buf,head->buf_len)==NULL)ERROR_EXIT_;
			request.recv_data=head->buf;
			request.recv_len=head->buf_len;

			i=connect->fd;
			pool_Free(head->buf,&share->head_pool);
			hash_Remove(&share->fd_list,head);
			connect->fd=i;
			/*find a host*/
			for(i=0;i<array_Length(share->host_list);++i)
			{
				ERROR_OUT_(stderr,ENCODE_("CHECK THE HOST:%s\n"),request.host);
				if(request.host!=NULL&&(!regexec(&share->host_list[i].preg,request.host,1,&preg_match,0)))
				{
					/*find a mod to use it*/
					return_state=0;
					for(j=0;j<array_Length(share->host_list[i].mod_config.mod_table);++j)
					{
						ERROR_OUT_(stderr,ENCODE_("START SELECT,THE FD IS:%d\n"),connect->fd);
						state=0;
						state=share->host_list[i].mod_config.mod_table[j].mod_Select(share->host_list[i].mod_config.mod_table[j].share,&request,connect->fd);
						if(state&SELECT_GOON_)continue;
						else
						{
							if(state&SELECT_BREAK_)
							{
								request_Free(&request,share);
								return WORK_CLOSE_;
							};
							if((state&SELECT_INPUT_)||(state&SELECT_OUTPUT_))
							{
								request_Free(&request,share);
								connect->mod=&share->host_list[i].mod_config.mod_table[j];
								if(state&SELECT_INPUT_)return_state|=WORK_INPUT_;
								if(state&SELECT_OUTPUT_)return_state|=WORK_OUTPUT_;
								if(state&SELECT_NEWPORT_)return_state|=WORK_NEWPORT_;
								return return_state;
							};
						};
					};
					request_Free(&request,share);
					ERROR_OUT_(stderr,ENCODE_("NO MOD TO WORK?NO POSSIBLE\n"));
					return WORK_CLOSE_;
				};
			};	
			request_Free(&request,share);
			ERROR_OUT_(stderr,ENCODE_("NO HOST FOUND\n"));
			return WORK_CLOSE_;
		}
		else
		{
			/*there are still datas to recv*/
			head->buf_len+=share->buf_size;
#if 0
			if(head->buf_len>share->max_head)
			{
				/*header too large*/
				ERROR_OUT_(stderr,ENCODE_("http header too large\n"));
				request_Free(&request,share);
				return WORK_CLOSE_;
			};
#endif
		};
	};
fail_return:
	request_Free(&request,share);
	/*ERROR_PRINT_;*/
	return WORK_CLOSE_;
};

int head_Close(HEAD_SHARE *share,HTTP_CONNECT *connect)
{
	HEAD_FD fake_head;
	HEAD_FD *head;

	fake_head.fd=connect->fd;
	head=hash_Get(&share->fd_list,&fake_head);
	if(head!=NULL)hash_Remove(&share->fd_list,head);
	return 0;
};
