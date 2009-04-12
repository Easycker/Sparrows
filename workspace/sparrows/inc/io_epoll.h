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
#include <errno.h>
#include "request.h"
#include "responder.h"

typedef struct io_config
{
	UINT_ http_port;
	int io_buf;
}IO_CONFIG;

typedef enum io_state
{
	PREPARE=0,
	RECV=1,
	SEND=2
}IO_STATE;

typedef struct http_connect
{
	int fd;
	IO_STATE state;
	void *connect_id;
}HTTP_CONNECT;

IO_CONFIG* io_Config(IO_CONFIG *config,FILE *fp);

UINT_ connect_Tinyhash(HTTP_CONNECT *connect);

int main(int argc,char *argv[]);
