#include "request.h"

int httpd_Nrecv(C_STRING *data,int fd,size_t buf_len,size_t n)
{
	size_t i;
	size_t recv_len;
	size_t request_len;
	C_ARRAY char *data_ansi;
	char end_char;

	data_ansi=array_Create(sizeof(char));
	array_Resize(&data_ansi,buf_len);
	request_len=0;
	do
	{
		recv_len=read(fd,((char*)data_ansi)+request_len,buf_len);
		request_len+=recv_len;
		array_Head(data_ansi)->array_length+=recv_len;
		array_Resize(&data_ansi,array_Length(data_ansi)+request_len+1);
	}while((recv_len>=buf_len)&&n==0?1:(request_len<n));
	end_char='\0';
	array_Append(&data_ansi,&end_char);

	PRINTF_(ENCODE_("the main data is:%s\n"),data_ansi);

	array_Resize(data,0);
	if(string_Ansitowide(data,data_ansi)==NULL)goto fail_return;
	array_Drop(&data_ansi);
	return 0;

	fail_return:
	array_Drop(&data_ansi);
	return -1;
}; 
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
	string_Drop(request->path);
	string_Drop(request->host);
	string_Drop(request->accept);
	string_Drop(request->accept_charset);
	string_Drop(request->accept_encoding);
	string_Drop(request->accept_language);
	string_Drop(request->cookies);
	string_Drop(request->referer);
	string_Drop(request->user_agent);
	string_Drop(request->query_string);
	string_Drop(request->remote_addr);
	string_Drop(request->content_type);
	string_Drop(request->content_length);
};
