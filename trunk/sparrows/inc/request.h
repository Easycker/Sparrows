#ifndef REQUEST_H
#define REQUEST_H

#include <cda/c_hash.h>
#include <cda/c_string.h>
#include <cda/c_define.h>
#include <stdint.h>
#include "shttpd_type.h"

#define POST_HEAD_ ENCODE_("\r\n\r\n")

typedef struct head_fd
{
	int fd;
	C_ARRAY char *buf;
}HEAD_FD;

typedef struct head_share
{
	C_HASH fd_list;
	size_t buf_size;
	C_ARRAY MOD_T *mod_table;
}HEAD_SHARE;

HTTP_REQUEST* request_Create(HTTP_REQUEST *request);

void key_Fetch(C_STRING *tag,CHAR_ const *string,CHAR_ const *key);

HTTP_REQUEST* request_Head(HTTP_REQUEST *request,C_ARRAY CHAR_* const string);

void request_Free(HTTP_REQUEST *request);

UINT_ head_Tinyhash(HEAD_FD *fd);

BOOL_ head_Ensure(HEAD_FD *lhs,HEAD_FD *rhs);

void head_Free(HEAD_FD *fd);

HEAD_SHARE* head_Init(size_t buf_size,C_ARRAY MOD_T *mod_table);

int head_Work(HEAD_SHARE *share,HTTP_CONNECT *connect);

#endif
