#ifndef IO_EPOLL_H
#define IO_EPOLL_H

#include <cda/c_hash.h>
#include <xmlo/xmlo.h>
#include <cda/c_mem.h>
#include <cda/c_string.h>
#include <cda/c_dchain.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <dlfcn.h>
#include <regex.h>
#include <time.h>
#include <errno.h>
#include "request.h"

typedef struct io_config
{
	struct sockaddr_in addr;
	UINT_ http_port;
	int io_buf;
	int pool_length;
	int pool_timeout;
	int keep_alive;
	int timeout;
	UINT_ max_head;
	C_ARRAY HOST_TYPE *host_list;
}IO_CONFIG;

IO_CONFIG* io_Init(IO_CONFIG *config,FILE *fp);

UINT_ connect_Tinyhash(HTTP_CONNECT **connect);

BOOL_ connect_Ensure(HTTP_CONNECT **lhs,HTTP_CONNECT **rhs);

int fd_Setnonblocking(int fd);

HTTP_CONNECT* event_Add(C_HASH *table,C_DCHAIN *chain,int epoll_fd,int fd,uint32_t event,MOD_T *mod);

void event_Mod(int epoll_fd,int fd,uint32_t event);

void event_Active(C_HASH *table,C_DCHAIN *connect_chain,HTTP_CONNECT *connect,HTTP_CONNECT **timeout,int epoll_fd);

C_HASH* event_Delete(C_HASH *table,C_DCHAIN *chain,HTTP_CONNECT *connect,int epoll_fd);

int epoll_Loop(C_HASH *connect_list,C_DCHAIN *connect_chain,int epoll_fd,IO_CONFIG *config,int listenfd);

int main(int argc,char *argv[]);

#endif
