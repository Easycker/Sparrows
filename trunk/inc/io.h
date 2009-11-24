#ifndef IO_H
#define IO_H

#include "shttpd_type.h"
#include "request.h"

#ifdef USE_SELECT
#include "io_select.h"
#endif

#ifdef USE_EPOLL
#include "io_epoll.h"
#endif

#include "static.h"
#include "proxy.h"
#include "rewrite.h"
#include "cgi.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <regex.h>
#include <time.h>
#include <errno.h>

IO_CONFIG* io_Init(IO_CONFIG *config,FILE *fp);

UINT_ connect_Tinyhash(HTTP_CONNECT **connect);

BOOL_ connect_Ensure(HTTP_CONNECT **lhs,HTTP_CONNECT **rhs);

int fd_Setnonblocking(int fd);

HTTP_CONNECT* event_Add(C_HASH *table,C_DLIST *list,struct event_pool *pool,int fd,uint32_t event,MOD_T *mod);

void event_Mod(struct event_pool *pool,int fd,uint32_t event);

void event_Active(C_HASH *table,C_DLIST *connect_list,HTTP_CONNECT *connect);

C_HASH* event_Delete(C_HASH *table,C_DLIST *list,HTTP_CONNECT *connect,struct event_pool *pool);

int event_Loop(C_HASH *connect_table,C_DLIST *connect_list,struct event_pool *pool,IO_CONFIG *config,int listenfd);

int main(int argc,char *argv[]);

#endif
