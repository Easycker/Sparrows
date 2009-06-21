#ifndef REQUEST_H
#define REQUEST_H

#include <cda/c_hash.h>
#include <cda/c_string.h>
#include <cda/c_define.h>
#include <stdint.h>
#include <errno.h>
#include "shttpd_type.h"

#define POST_HEAD_ ENCODE_("\r\n\r\n")

typedef struct head_fd
{
	int fd;
	int buf_len;
	char *buf;
}HEAD_FD;

typedef struct head_share
{
	C_HASH fd_list;
	size_t buf_size;
	UINT_ max_head;
	C_POOL head_pool;
	C_ARRAY HOST_TYPE *host_list;
}HEAD_SHARE;

HTTP_REQUEST* request_Create(HTTP_REQUEST *request);

C_STRING key_Fetch(C_STRING *tag,CHAR_ const *string,CHAR_ const *key);

HTTP_REQUEST* request_Head(HTTP_REQUEST *request,C_ARRAY CHAR_* const string);

void request_Free(HTTP_REQUEST *request,HEAD_SHARE *head_share);

UINT_ head_Tinyhash(HEAD_FD *fd);

BOOL_ head_Ensure(HEAD_FD *lhs,HEAD_FD *rhs);

void head_Free(HEAD_FD *fd);

HEAD_SHARE* head_Init(size_t buf_size,UINT_ max_head,C_ARRAY HOST_TYPE *host_list);

int head_Work(HEAD_SHARE *share,HTTP_CONNECT *connect);

int head_Close(HEAD_SHARE *share,HTTP_CONNECT *connect);

#endif
