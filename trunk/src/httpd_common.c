#include "httpd_common.h"

C_STRING httpd_Nrecv(C_STRING *data,int fd,int buf_len,size_t n)
{
	size_t i;
	int recv_len;
	size_t request_len;
	C_ARRAY char *data_ansi;

	data_ansi=array_Create(sizeof(char));
	data_ansi=array_Resize(data_ansi,buf_len);
	do
	{
		recv_len=read(fd,((char*)data_ansi)+request_len,buf_len);
		request_len+=recv_len;
		array_Head(data_ansi)->array_length+=recv_len;
		data_ansi=array_Resize(data_ansi,array_Length(data_ansi)+request_len+1);
	}while((int)recv_len>=(int)(buf_len));

	*data=array_Resize(*data,0);
	if(string_Ansitowide(data,data_ansi)==NULL)goto fail_return;
	array_Drop(data_ansi);
	return *data;

	fail_return:
	array_Drop(data_ansi);
	return NULL;
};