#ifndef REQUEST_H
#define REQUEST_H

#include "shttpd_type.h"
#include <stdint.h>
#include <sys/socket.h>
#include <errno.h>

#define POST_HEAD_ "\r\n\r\n"

#define ARG_LEN_ 256

#define ARG_STR 0x0000000f
#define ARG_BOOL 0x000000f0

typedef struct head_fd
{
	int fd;
	size_t buf_len;
	char *buf;
}HEAD_FD;

typedef struct arg_node
{
	C_STRING name;
	int type;
	intptr_t offset;
}ARG_NODE;

typedef struct head_share
{
	C_HASH fd_list;
	size_t buf_size;
	UINT_ max_head;
	C_BHTREE arg_tree;
	C_POOL head_pool;
	C_ARRAY HOST_TYPE *host_list;
}HEAD_SHARE;

BOOL_ arg_Comp(void *lhs,void *rhs);

BOOL_ arg_Equal(void *lhs,void *rhs);

HTTP_REQUEST* request_Create(HTTP_REQUEST *request);

HTTP_REQUEST* request_Head(C_BHTREE *tree,HTTP_REQUEST *request,CHAR_* const string,size_t length);

void request_Free(HTTP_REQUEST *request,HEAD_SHARE *head_share);

UINT_ head_Tinyhash(HEAD_FD *fd);

BOOL_ head_Ensure(HEAD_FD *lhs,HEAD_FD *rhs);

void head_Free(HEAD_FD *fd);

HEAD_SHARE* head_Init(size_t buf_size,UINT_ max_head,C_ARRAY HOST_TYPE *host_list);

int head_Work(HEAD_SHARE *share,HTTP_CONNECT *connect);

int head_Close(HEAD_SHARE *share,HTTP_CONNECT *connect);

#endif
