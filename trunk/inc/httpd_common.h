#ifndef HTTPD_COMMON_H
#define HTTPD_COMMON_H

#include <cda/c_string.h>

C_STRING httpd_Nrecv(C_STRING *data,int fd,int buf_len,size_t n);

#endif