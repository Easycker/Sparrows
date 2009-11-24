#ifndef PROXY_H
#define PROXY_H

#include "shttpd_type.h"
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <regex.h>
#include <errno.h>

#define PROXY_NAME "proxy"

#define NOTHING_ 0
#define CONNECTING_ 1
#define LOCAL_SEND_REMOTE_ 2
#define REMOTE_SEND_LOCAL_ 3

#ifndef SPLICE_F_NONBLOCK
#define SPLICE_F_NONBLOCK 2
#endif

typedef struct proxy_regex
{
	regex_t preg;
	struct sockaddr_in addr;
}PROXY_REGEX;

typedef struct proxy_config
{
	C_ARRAY PROXY_REGEX *regex_list;
	int buf_size;
	int new_fd;
	int close_fd;
	C_HASH id_list;
	C_DLIST owner_list;
}PROXY_CONFIG;

typedef struct proxy_owner
{
	int in_fd;
	int remote_fd;
	int buf_fd[2];
	int last_op;
	int len;
	BOOL_ in_read;
	BOOL_ remote_read;
	BOOL_ in_write;
	BOOL_ remote_write;
}PROXY_OWNER;

typedef struct proxy_id
{
	int fd;
	PROXY_OWNER *owner;
}PROXY_ID;

UINT_ proxy_Tinyhash(PROXY_ID *id);

BOOL_ proxy_Ensure(PROXY_ID *lhs,PROXY_ID *rhs);

int fd_Setnonblocking(int fd);

/*this is the main function*/
PROXY_CONFIG* proxy_Init(IO_CONFIG *io_config,CHAR_ const *arg);

int proxy_Select(PROXY_CONFIG *config,HTTP_REQUEST *request,int fd);

int proxy_Work(PROXY_CONFIG *config,HTTP_CONNECT *connect);

int proxy_Addport(PROXY_CONFIG *config,HTTP_CONNECT *connect,PORT_APPLY *apply);

int proxy_Closeport(PROXY_CONFIG *config,HTTP_CONNECT *connect,PORT_APPLY *apply);

int proxy_Close(PROXY_CONFIG *config,HTTP_CONNECT *connect);

void proxy_Unload(PROXY_CONFIG *config);

#endif
