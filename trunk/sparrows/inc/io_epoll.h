#ifndef IO_EPOLL_H
#define IO_EPOLL_H

#include <xmlo/xmlo.h>
#include <cda/c_mem.h>
#include <cda/c_string.h>
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
#include <errno.h>
#include "request.h"
#include "modmanage.h"

typedef struct io_config
{
	UINT_ http_port;
	int io_buf;
	int poll_length;
}IO_CONFIG;

IO_CONFIG* io_Init(IO_CONFIG *config,FILE *fp);

UINT_ connect_Tinyhash(HTTP_CONNECT *connect);

BOOL_ connect_Ensure(HTTP_CONNECT *lhs,HTTP_CONNECT *rhs);

int fd_Setnonblocking(int fd);

C_HASH* event_Add(C_HASH *table,int epoll_fd,int fd,uint32_t event,MOD_T *mod);

void event_Mod(int epoll_fd,int fd,uint32_t event);

C_HASH* event_Delete(C_HASH *table,HTTP_CONNECT *connect,int epoll_fd,int fd);

int epoll_Loop(C_HASH *connect_list,int epoll_fd,IO_CONFIG *config,MODMANAGE_CONFIG *mod_config,int listenfd);

int main(int argc,char *argv[]);

#endif
