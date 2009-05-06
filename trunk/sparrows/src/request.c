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

	/*
	for(i=0;i<11;++i)if(((C_STRING*)(&request+sizeof(REQUEST_TYPE)+sizeof(C_STRING)*2+sizeof(BOOL_)))[i]==NULL)
	{
		for(i=0;i<11;++i)if(((C_STRING*)(&request+sizeof(REQUEST_TYPE)+sizeof(C_STRING)*2+sizeof(BOOL_)))[i]!=NULL)string_Drop(&((C_STRING*)(&request+sizeof(REQUEST_TYPE)+sizeof(C_STRING)*2+sizeof(BOOL_)))[i]);
		return NULL;
	};
	*/

	return request;
};

C_STRING key_Fetch(C_STRING *tag,CHAR_ const *string,CHAR_ const *key)
{
	KMP_KEY search_key;
	CHAR_ *op;
	CHAR_ end_char;
	KMP_KEY *kres;

	end_char=ENCODE_('\0');
	if(string_Kprepare(&search_key,key)==NULL)
	{
		array_Resize(tag,0);
		return NULL;
	};
	op=string_Knsearch(string,&search_key,array_Length(string));
	string_Kfree(&search_key);
	if(op!=NULL)
	{
		array_Resize(tag,0);
		for(op+=STRLEN_(key);*op!=ENCODE_('\r');++op)array_Append(tag,op);
		array_Append(tag,&end_char);
	}
	else
	{
		array_Resize(tag,0);
	};
	return *tag;

};

HTTP_REQUEST* request_Head(HTTP_REQUEST *request,C_ARRAY CHAR_* const string)
{
	CHAR_ *op;
	int i;
	CHAR_ end_char;
	KMP_KEY search_key;

	i=0;
	end_char=ENCODE_('\0');
	op=string;
	if(*op==ENCODE_('G'))request->type=GET;
	else if(*op==ENCODE_('P'))request->type=POST;
	else goto fail_return;
	while(*op!=ENCODE_(' '))++op;
	array_Resize(&request->path,0);
	for(++op;*op!=ENCODE_('?')&&*op!=ENCODE_(' ');++op)array_Append(&request->path,op);
	array_Append(&request->path,&end_char);
	array_Resize(&request->query_string,0);
	if(*op==ENCODE_('?'))
	{
		for(++op;*op!=ENCODE_(' ');++op)if(array_Append(&request->query_string,op)==NULL)goto fail_return;
		if(array_Append(&request->query_string,&end_char)==NULL)goto fail_return;
	};

	/*now start to check Host and Keep-alive*/
	/*Host*/
	if(string_Kprepare(&search_key,ENCODE_("Host: "))==NULL)goto fail_return;
	op=string_Knsearch(string,&search_key,array_Length(string));
	string_Kfree(&search_key);
	if(op!=NULL)
	{
		array_Resize(&request->host,0);
		for(op+=6;*op!=ENCODE_('\r');++op)if(array_Append(&request->host,op)==NULL)goto fail_return;
		if(array_Append(&request->host,&end_char)==NULL)goto fail_return;
	};

	/*Connection*/
	if(string_Kprepare(&search_key,ENCODE_("Connection: "))==NULL)goto fail_return;
	op=string_Knsearch(string,&search_key,array_Length(string));
	string_Kfree(&search_key);
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
	return NULL;
};

void request_Free(HTTP_REQUEST *request)
{
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
	/*
	uint32_t hash=0;
	uint32_t x=0;
	int i;
	char *op;
	size_t len;

	len=sizeof(head->fd);
	op=(char*)&(head->fd);
	for(i=0;i<len;++i)
	{
		hash=(hash<<4)+(*op++);
		if((x=hash&0xF0000000L)!=0)
		{
			hash^= (x>>24);
			hash&=~x;
		};
	};
	return (hash&HASH_SPACE_);
	*/
	return head->fd&HASH_SPACE_;
};

BOOL_ head_Ensure(HEAD_FD *lhs,HEAD_FD *rhs)
{
	ERROR_OUT_(stderr,ENCODE_("I AN ENSURE,MY LHS IS:%d,RHS IS %d\n"),lhs->fd,rhs->fd);
	return lhs->fd==rhs->fd;
};

void head_Free(HEAD_FD *fd)
{
	array_Drop(&fd->buf);
};

HEAD_SHARE* head_Init(size_t buf_size,C_ARRAY MOD_T *mod_table)
{
	HEAD_SHARE *share;

	share=(HEAD_SHARE*)malloc(sizeof(HEAD_SHARE));
	if(share==NULL)
	{
		ERROR_OUT_(stderr,ENCODE_("SHARE IS NULL\n"));
		return NULL;
	};
	hash_Create_Ex(&share->fd_list,&head_Tinyhash,&head_Ensure,sizeof(HEAD_FD),(UINT_)HASH_SPACE_,&head_Free);
	share->buf_size=buf_size;
	share->mod_table=mod_table;
};

int head_Work(HEAD_SHARE *share,HTTP_CONNECT *connect)
{
	HEAD_FD *head;
	HEAD_FD fake_head;
	C_STRING head_data;
	HTTP_REQUEST request;
	int state;
	int return_state;
	int len;
	int i;

	fake_head.fd=connect->fd;
	if(&share->fd_list==NULL||&fake_head==NULL)ERROR_OUT_(ENCODE_("null\n"));
	head=hash_Get(&share->fd_list,&fake_head);
	if(head==NULL)
	{
		ERROR_OUT_(stderr,ENCODE_("HERE CREATE A NEW HEAD\n"));
		fake_head.fd=connect->fd;
		fake_head.buf=array_Create(sizeof(char));
		array_Resize(&fake_head.buf,share->buf_size);
		hash_Append(&share->fd_list,&fake_head);
		head=hash_Get(&share->fd_list,&fake_head);
	};
	array_Resize(&head->buf,array_Head(head->buf)->array_space+share->buf_size);
	while(1)
	{
		len=recv(head->fd,&head->buf[array_Length(head->buf)],share->buf_size,0);
		if(len<0)
		{
			if(errno==EAGAIN)
			{
				array_Length(head->buf)+=share->buf_size;
				array_Head(head->buf)->array_space+=share->buf_size;
				return WORK_GOON_;
			}
			else
			{
				return WORK_CLOSE_;
			};
		}
		else if(len<share->buf_size)
		{
			ERROR_OUT_(stderr,ENCODE_("HEAD RECV DONE,THE FD IS;%d\n"),connect->fd);
			/*all data had been recv*/
			array_Length(head->buf)+=len;
			head->buf[array_Length(head->buf)]='\0';
			head_data=string_Create();
			if(head_data==NULL)goto fail_return;
			if(string_Ansitowide(&head_data,head->buf)==NULL)goto fail_return;
			/*anlysis the head*/
			if(request_Create(&request)==NULL)goto fail_return;
			if(request_Head(&request,head_data)==NULL)goto fail_return;
			/*release the resources*/
			string_Drop(&head_data);
			/*array_Drop(&head->buf);*/
			i=connect->fd;
			hash_Remove(&share->fd_list,head);
			connect->fd=i;
			/*find a mod to use it*/
			/* here is a test*/
			return_state=0;
			for(i=0;i<array_Length(share->mod_table);++i)
			{
				ERROR_OUT_(stderr,ENCODE_("START SELECT,THE FD IS:%d\n"),connect->fd);
				state=share->mod_table[i].mod_Select(share->mod_table[i].share,&request,connect->fd);
				if(state&SELECT_GOON_)continue;
				else
				{
					if(state&SELECT_BREAK_)
					{
						request_Free(&request);
						return WORK_CLOSE_;
					};
					if((state&SELECT_INPUT_)||(state&SELECT_OUTPUT_))
					{
						request_Free(&request);
						connect->mod=&share->mod_table[i];
						if(state&SELECT_INPUT_)return_state|=WORK_INPUT_;
						else return_state|=WORK_OUTPUT_;
						if(state&SELECT_NEWPORT_)return_state|=WORK_NEWPORT_;
						return return_state;
					};
				};
			};
			request_Free(&request);
			ERROR_OUT_(stderr,ENCODE_("NO MOD TO WORK?NO POSSIBLE\n"));
			return WORK_CLOSE_;
		}
		else
		{
			/*there are still datas to recv*/
			array_Length(head->buf)+=share->buf_size;
			array_Head(head->buf)->array_space+=share->buf_size;
		};
	};
fail_return:
	return WORK_CLOSE_;
};
