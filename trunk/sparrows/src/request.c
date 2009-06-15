#include "request.h"

HTTP_REQUEST* request_Create(HTTP_REQUEST *request)
{
	int i;

	request->path=string_Create();
	request->host=string_Create();
	request->accept=string_Create();
	request->accept_charset=string_Create();
	request->accept_encoding=string_Create();
	request->accept_language=string_Create();
	request->cookies=string_Create();
	request->referer=string_Create();
	request->user_agent=string_Create();
	request->query_string=string_Create();
	request->remote_addr=string_Create();
	request->content_type=string_Create();
	request->content_length=string_Create();

	return request;
};

C_STRING key_Fetch(C_STRING *tag,CHAR_ const *string,CHAR_ const *key)
{
	KMP_KEY search_key;
	CHAR_ *op;
	KMP_KEY *kres;

	if(string_Kprepare(&search_key,key)==NULL)
	{
		array_Resize(tag,0);
		return NULL;
	};
	op=string_Knsearch(string,&search_key,array_Length(string));
	string_Kfree(&search_key);
	array_Resize(tag,0);
	if(op!=NULL)
	{
		for(op+=STRLEN_(key);*op!=ENCODE_('\r');++op)string_Append(tag,op);
	};
	return *tag;
};

HTTP_REQUEST* request_Head(HTTP_REQUEST *request,C_ARRAY CHAR_* const string)
{
	CHAR_ *op;
	int i;
	KMP_KEY search_key;

	i=0;
	op=string;
	if(*op==ENCODE_('G'))request->type=GET;
	else if(*op==ENCODE_('P'))request->type=POST;
	else goto fail_return;
	while(*op!=ENCODE_(' '))++op;
	array_Resize(&request->path,0);
	for(++op;*op!=ENCODE_('?')&&*op!=ENCODE_(' ');++op)string_Append(&request->path,op);
	array_Resize(&request->query_string,0);
	if(*op==ENCODE_('?'))
	{
		for(++op;*op!=ENCODE_(' ');++op)if(string_Append(&request->query_string,op)==NULL)goto fail_return;
	};

	/*now start to check Host and Keep-alive*/
	/*Host*/
	if(string_Kprepare(&search_key,ENCODE_("Host: "))==NULL)goto fail_return;
	op=string_Knsearch(string,&search_key,array_Length(string));
	string_Kfree(&search_key);
	if(op!=NULL)
	{
		array_Resize(&request->host,0);
		for(op+=6;*op!=ENCODE_('\r');++op)if(string_Append(&request->host,op)==NULL)goto fail_return;
	};

	/*Connection*/
	if(string_Kprepare(&search_key,ENCODE_("Connection: "))==NULL)goto fail_return;
	op=string_Knsearch(string,&search_key,array_Length(string));
	string_Kfree(&search_key);
	request->alive=FALSE_;
	if(op!=NULL)
	{
		if(*(op+12)==ENCODE_('K'))request->alive=TRUE_;
		else request->alive=FALSE_;
		ERROR_OUT_(stderr,ENCODE_("KEEP ALIVE:%d\n"),request->alive);
	};

	/*common cgi vars*/
	/*if it's a post,get the type and length*/
	if(request->type==POST)
	{
		if(key_Fetch(&request->content_type,string,ENCODE_("Content-Type: "))==NULL)goto fail_return;
		if(key_Fetch(&request->content_length,string,ENCODE_("Content-Length: "))==NULL)goto fail_return;
	};
	/*accept*/
	if(key_Fetch(&request->accept,string,ENCODE_("Accept: "))==NULL)goto fail_return;

	if(key_Fetch(&request->accept_charset,string,ENCODE_("Accept-Charset: "))==NULL)goto fail_return;
	if(key_Fetch(&request->accept_encoding,string,ENCODE_("Accept-Encoding: "))==NULL)goto fail_return;
	if(key_Fetch(&request->accept_language,string,ENCODE_("Accept-Language: "))==NULL)goto fail_return;
	if(key_Fetch(&request->cookies,string,ENCODE_("Cookies: "))==NULL)goto fail_return;
	if(key_Fetch(&request->referer,string,ENCODE_("Referer: "))==NULL)goto fail_return;
	if(key_Fetch(&request->user_agent,string,ENCODE_("User-Agent: "))==NULL)goto fail_return;
	
	return request;

	fail_return:
	ERROR_PRINT_;
	return NULL;
};

void request_Free(HTTP_REQUEST *request,HEAD_SHARE *head_share)
{
	pool_Free(request->recv_data,&(head_share->head_pool));

	string_Drop(&request->path);
	string_Drop(&request->host);
	string_Drop(&request->accept);
	string_Drop(&request->accept_charset);
	string_Drop(&request->accept_encoding);
	string_Drop(&request->accept_language);
	string_Drop(&request->cookies);
	string_Drop(&request->referer);
	string_Drop(&request->user_agent);
	string_Drop(&request->query_string);
	string_Drop(&request->remote_addr);
	string_Drop(&request->content_type);
	string_Drop(&request->content_length);
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
};

HEAD_SHARE* head_Init(size_t buf_size,UINT_ max_head,C_ARRAY HOST_TYPE *host_list)
{
	HEAD_SHARE *share;

	share=(HEAD_SHARE*)malloc(sizeof(HEAD_SHARE));
	if(share==NULL)
	{
		ERROR_OUT_(stderr,ENCODE_("SHARE IS NULL\n"));
		return NULL;
	};
	share->max_head=max_head;
	hash_Create_Ex(&share->fd_list,&head_Tinyhash,&head_Ensure,sizeof(HEAD_FD),(UINT_)HASH_SPACE_,&head_Free);
	pool_Create(&(share->head_pool),share->max_head);
	share->buf_size=buf_size;
	share->host_list=host_list;
	return share;
};

int head_Work(HEAD_SHARE *share,HTTP_CONNECT *connect)
{
	HEAD_FD *head;
	HEAD_FD fake_head;
	C_STRING head_data;
	HTTP_REQUEST request;
	C_ARRAY char *cache_ansi;
	regmatch_t preg_match;
	int state;
	int return_state;
	int len;
	int i;
	int j;

	cache_ansi=array_Create(sizeof(char));
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
		if(len<0)
		{
			if(errno==EAGAIN)
			{
				head->buf_len+=share->buf_size;
				array_Drop(&cache_ansi);
				return WORK_GOON_;
			}
			else
			{
				array_Drop(&cache_ansi);
				return WORK_CLOSE_;
			};
		}
		else if(len<share->buf_size)
		{
			ERROR_OUT_(stderr,ENCODE_("HEAD RECV DONE,THE FD IS;%d\n"),connect->fd);
			/*all data had been recv*/
			head->buf_len+=len;
			head->buf[head->buf_len]='\0';
			head_data=string_Create();
			if(head_data==NULL)goto fail_return;
			if(string_Ansitowide(&head_data,head->buf)==NULL)goto fail_return;
			/*anlysis the head*/
			if(request_Create(&request)==NULL)goto fail_return;
			request.recv_data=head->buf;
			request.recv_len=head->buf_len;
			if(request_Head(&request,head_data)==NULL)goto fail_return;
			/*release the resources*/
			string_Drop(&head_data);
			i=connect->fd;
			hash_Remove(&share->fd_list,head);
			connect->fd=i;
			/*find a host*/
			for(i=0;i<array_Length(share->host_list);++i)
			{
				ERROR_OUT_(stderr,ENCODE_("CHECK THE HOST:%S\n"),request.host);
				if(!regexec(&share->host_list[i].preg,string_Widetoansi(&cache_ansi,request.host),1,&preg_match,0))
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
								array_Drop(&cache_ansi);
								return WORK_CLOSE_;
							};
							if((state&SELECT_INPUT_)||(state&SELECT_OUTPUT_))
							{
								request_Free(&request,share);
								connect->mod=&share->host_list[i].mod_config.mod_table[j];
								if(state&SELECT_INPUT_)return_state|=WORK_INPUT_;
								if(state&SELECT_OUTPUT_)return_state|=WORK_OUTPUT_;
								if(state&SELECT_NEWPORT_)return_state|=WORK_NEWPORT_;
								array_Drop(&cache_ansi);
								return return_state;
							};
						};
					};
					request_Free(&request,share);
					array_Drop(&cache_ansi);
					ERROR_OUT_(stderr,ENCODE_("NO MOD TO WORK?NO POSSIBLE\n"));
					return WORK_CLOSE_;
				};
			};	
			request_Free(&request,share);
			array_Drop(&cache_ansi);
			ERROR_OUT_(stderr,ENCODE_("NO HOST FOUND\n"));
			return WORK_CLOSE_;
		}
		else
		{
			/*there are still datas to recv*/
			head->buf_len+=share->buf_size;
			if(head->buf_len>share->max_head)
			{
				/*header too large*/
				ERROR_OUT_(stderr,ENCODE_("http header too large\n"));
				request_Free(&request,share);
				array_Drop(&cache_ansi);
				return WORK_CLOSE_;
			};
		};
	};
fail_return:
	array_Drop(&cache_ansi);
	ERROR_PRINT_;
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
