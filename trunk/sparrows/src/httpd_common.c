#include "httpd_common.h"

C_STRING httpd_Nrecv(C_STRING *data,int fd,int buf_len,size_t n)
{
	
	size_t i;
	int recv_len;
	size_t request_len;
	C_ARRAY char *data_ansi;
	char end_char;

	data_ansi=array_Create(sizeof(char));
	data_ansi=array_Resize(data_ansi,buf_len);
	request_len=0;
	do
	{
		recv_len=read(fd,((char*)data_ansi)+request_len,buf_len);
		request_len+=recv_len;
		array_Head(data_ansi)->array_length+=recv_len;
		data_ansi=array_Resize(data_ansi,array_Length(data_ansi)+request_len+1);
	}while((int)recv_len>=(int)(buf_len));
	end_char='\0';
	data_ansi=array_Append(data_ansi,&end_char);

	*data=array_Resize(*data,0);
	if(string_Ansitowide(data,data_ansi)==NULL)goto fail_return;
	array_Drop(data_ansi);
	return *data;

	fail_return:
	array_Drop(data_ansi);
	return NULL;

};

HTTP_REQUEST* request_Create(HTTP_REQUEST *request)
{
	request->path=string_Create();
	request->get=string_Create();
	request->post=string_Create();
	return request;
};

HTTP_REQUEST* request_Analysis(HTTP_REQUEST *request,C_ARRAY CHAR_* const string)
{
	CHAR_ *op;
	int i;
	CHAR_ end_char;

	i=0;
	end_char=ENCODE_('\0');
	op=string;
	if(*op==ENCODE_('G'))request->type=GET;
	else if(*op==ENCODE_('P'))request->type=POST;
	else goto fail_return;
	while(*op!=ENCODE_(' '))++op;
	request->path=array_Resize(request->path,0);
	for(++op;*op!=ENCODE_('?')&&*op!=ENCODE_(' ');++op)request->path=array_Append(request->path,op);
	request->path=array_Append(request->path,&end_char);
	request->get=array_Resize(request->get,0);
	if(*op==ENCODE_('?'))for(;*op!=ENCODE_(' ');++op)request->get=array_Append(request->get,op);
	request->get=array_Append(request->get,&end_char);
	return request;

	fail_return:
	return NULL;
};

BOOL_ request_Send(HTTP_REQUEST *request,int fd)
{
	
};

unsigned char elf_Tinyhash(C_STRING const string)
{
	uint32_t hash=0;
	uint32_t x=0 ;
	char *op;
	size_t len;

	len=(array_Length(string)+1)*sizeof(CHAR_);
	op=(char*)string;
	while(len--)
	{
		hash=(hash<<4)+(*op++);
		if((x=hash&0xF0000000L)!=0)
		{
			hash^= (x>>24);
			hash&=~x;
		};
	};
	return (unsigned char)(hash&255);
};