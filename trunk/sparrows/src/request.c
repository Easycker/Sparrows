#include "request.h"

HTTP_REQUEST* request_Create(HTTP_REQUEST *request)
{
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

void key_Fetch(C_STRING *tag,CHAR_ const *string,CHAR_ const *key)
{
	KMP_KEY search_key;
	CHAR_ *op;
	CHAR_ end_char;

	end_char=ENCODE_('\0');
	string_Kprepare(&search_key,key);
	op=string_Knsearch(string,&search_key,array_Length(string));
	string_Kfree(&search_key);
	if(op!=NULL)
	{
		array_Resize(tag,0);
		for(op+=STRLEN_(key);*op!=ENCODE_('\r');++op)array_Append(tag,op);
		array_Append(tag,&end_char);
		PRINTF_(ENCODE_("OK,THE %S IS %S\n"),key,*tag);
	}
	else
	{
		array_Resize(tag,0);
	};

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
	ERROR_OUT_(stderr,ENCODE_("SOLVE THE PATH,IS %S\n"),request->path);
	array_Resize(&request->query_string,0);
	if(*op==ENCODE_('?'))
	{
		for(++op;*op!=ENCODE_(' ');++op)array_Append(&request->query_string,op);
		array_Append(&request->query_string,&end_char);
	};

	/*now start to check Host and Keep-alive*/
	/*Host*/
	string_Kprepare(&search_key,ENCODE_("Host: "));
	op=string_Knsearch(string,&search_key,array_Length(string));
	string_Kfree(&search_key);
	if(op!=NULL)
	{
		array_Resize(&request->host,0);
		for(op+=6;*op!=ENCODE_('\r');++op)array_Append(&request->host,op);
		array_Append(&request->host,&end_char);
		PRINTF_(ENCODE_("OK,THE HOST IS %S\n"),request->host);
	};

	/*Connection*/
	string_Kprepare(&search_key,ENCODE_("Connection: "));
	op=string_Knsearch(string,&search_key,array_Length(string));
	string_Kfree(&search_key);
	if(op!=NULL)
	{
		if(*(op+12)==ENCODE_('K'))request->alive=TRUE_;
		else request->alive=FALSE_;
		PRINTF_(ENCODE_("KEEP ALIVE:%d\n"),request->alive);
	};

	/*common cgi vars*/
	/*if it's a post,get the type and length*/
	if(request->type==POST)
	{
		key_Fetch(&request->content_type,string,ENCODE_("Content-Type: "));
		key_Fetch(&request->content_length,string,ENCODE_("Content-Length: "));
	};
	/*accept*/
	key_Fetch(&request->accept,string,ENCODE_("Accept: "));

	key_Fetch(&request->accept_charset,string,ENCODE_("Accept-Charset: "));
	key_Fetch(&request->accept_encoding,string,ENCODE_("Accept-Encoding: "));
	key_Fetch(&request->accept_language,string,ENCODE_("Accept-Language: "));
	key_Fetch(&request->cookies,string,ENCODE_("Cookies: "));
	key_Fetch(&request->referer,string,ENCODE_("Referer: "));
	key_Fetch(&request->user_agent,string,ENCODE_("User-Agent: "));
	
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

UINT_ head_Tinyhash(HEAD_FD *fd)
{
	uint32_t hash=0;
	uint32_t x=0;
	int i;
	char *op;
	size_t len;

	len=sizeof(fd->fd);
	op=(char*)&(fd->fd);
	for(i=0;i<len;++i)
	{
		hash=(hash<<4)+(*op++);
		if((x=hash&0xF0000000L)!=0)
		{
			hash^= (x>>24);
			hash&=~x;
		};
	};
	return (hash&0x0000ffff);
};

BOOL_ head_Ensure(HEAD_FD *lhs,HEAD_FD *rhs)
{
	return lhs->fd==rhs->fd;
};

void head_Free(HEAD_FD *fd)
{
};

HEAD_SHARE* head_Init(size_t buf_size,C_ARRAY MOD_T *mod_table)
{
	HEAD_SHARE *share;

	share=(HEAD_SHARE*)malloc(sizeof(HEAD_SHARE));
	hash_Create_Ex(&share->fd_list,&head_Tinyhash,&head_Ensure,sizeof(HEAD_FD),0x0000ffff,&head_Free);
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
	head=hash_Get(&share->fd_list,&fake_head);
	if(head==NULL)
	{
		fake_head.fd=connect->fd;
		fake_head.buf=array_Create(sizeof(char));
		array_Resize(&fake_head.buf,share->buf_size);
		hash_Append(&share->fd_list,&fake_head);
		head=hash_Get(&share->fd_list,&fake_head);
	};
	array_Resize(&head->buf,array_Head(head->buf)->array_space+share->buf_size);
	len=recv(head->fd,&head->buf[array_Length(head->buf)],share->buf_size,0);
	if(len<0)
	{
		/*must be some error here,i am using epoll lt,recv a event meant that the fd should be writable or readable*/
		PRINTF_(ENCODE_("GOT HEAD FAIL\n"));
		return WORK_CLOSE_;
	}
	else if(len<share->buf_size)
	{
		/*all data had been recv*/
		array_Length(head->buf)+=len;
		head->buf[array_Length(head->buf)]='\0';
		head_data=string_Create();
		string_Ansitowide(&head_data,head->buf);
		ERROR_OUT_(stderr,ENCODE_("RECV THE HEAD IS:\n%S\n"),head_data);
		/*anlysis the head*/
		request_Create(&request);
		request_Head(&request,head_data);
		ERROR_OUT_(stderr,ENCODE_("PATH IS %S\n"),request.path);
		/*release the resources*/
		string_Drop(&head_data);
		array_Drop(&head->buf);
		hash_Remove(&share->fd_list,head);
		/*find a mod to use it*/
		/* here is a test*/
		return_state=0;
		for(i=0;i<array_Length(share->mod_table);++i)
		{
			state=share->mod_table[i].mod_Select(share->mod_table[i].share,&request,connect->fd);
			if(state&SELECT_GOON_)continue;
			else
			{
				if(state&SELECT_GOON_)continue;
				if(state&SELECT_BREAK_)return WORK_CLOSE_;
				if((state&SELECT_INPUT_)||(state&SELECT_OUTPUT_))
				{
					connect->mod=&share->mod_table[i];
					if(state&SELECT_INPUT_)return_state|=WORK_INPUT_;
					else return_state|=WORK_OUTPUT_;
					if(state&SELECT_NEWPORT_)return_state|=WORK_NEWPORT_;
					return return_state;
				};
			};
		};
		return WORK_CLOSE_;
	}
	else
	{
		PRINTF_(ENCODE_("STILL DATA TO RECV\n"));
		PRINTF_(ENCODE_("THE DATA IS:%s\n"),head->buf);
		/*there are still datas to recv*/
		array_Length(head->buf)+=share->buf_size;
		/*continue read,set the epoll*/
		return WORK_GOON_;
	};
};