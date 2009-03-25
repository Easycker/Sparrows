#ifndef HTTPD_COMMON_H
#define HTTPD_COMMON_H

#include <cda/c_string.h>
#include <cda/c_define.h>
#include <stdint.h>
#include "shttpd_type.h"

C_STRING httpd_Nrecv(C_STRING *data,int fd,size_t buf_len,size_t n);

void http_Nsend(CHAR_ const *data,int fd,int buf_len,size_t n);

HTTP_REQUEST* request_Create(HTTP_REQUEST *request);

HTTP_REQUEST* request_Analysis(HTTP_REQUEST *request,C_ARRAY CHAR_* const string);

BOOL_ request_Send(HTTP_REQUEST *request,int fd);

#endif