#ifndef IO_PROCESS_H
#define IO_PROCESS_H

/*Keep It Simple & Stupid*/
#include <xmlo/xmlo.h>
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

#define HOWTO_ ENCODE_("example:./server <filename>\n")

typedef struct io_config
{
	UINT_ max_process;
	UINT_ stable_process;
	UINT_ http_port;
	C_STRING lock_file;
	int resv_buf;
	int io_buf;
}IO_CONFIG;

typedef struct lock_t
{
	C_ARRAY char* file;
}LOCK_T;

typedef struct http_connect
{
	int fd;
	void *connect_id;
}HTTP_CONNECT;

IO_CONFIG* io_Config(IO_CONFIG *config,FILE *fp);

LOCK_T* lock_Init(LOCK_T *lock,C_ARRAY CHAR_* const filename);

BOOL_ lock_Get(LOCK_T *lock);

void lock_Free(LOCK_T *lock);

void child_Main(IO_CONFIG *config,RESPONDER_CONFIG *res_config,int listenfd,LOCK_T *child_lock);

void signal_Quit(int sig);

int main(int argc,char *argv[]);

#endif
