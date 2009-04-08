#ifndef REQUEST_H
#define REQUEST_H

#include <cda/c_string.h>
#include <cda/c_define.h>
#include <stdint.h>
#include "shttpd_type.h"

#define POST_HEAD_ ENCODE_("\r\n\r\n")

int httpd_Nrecv(C_STRING *data,int fd,size_t buf_len,size_t n);

HTTP_REQUEST* request_Create(HTTP_REQUEST *request);

void key_Fetch(C_STRING *tag,CHAR_ const *string,CHAR_ const *key);

HTTP_REQUEST* request_Head(HTTP_REQUEST *request,C_ARRAY CHAR_* const string);

void request_Free(HTTP_REQUEST *request);

#endif
