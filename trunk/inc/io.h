/*Keep It Simple & Stupid*/
#include <xmlo/xmlo.h>
#include <cda/c_string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <errno.h>
#include "httpd_common.h"

#define HOWTO_ ENCODE_("example:./server <filename>\n")

typedef struct io_config
{
	UINT_ max_process;
	UINT_ stable_process;
	UINT_ http_port;
	C_STRING cache_adr;
	C_STRING lock_file;
	UINT_ cache_port;
	int resv_buf;
}IO_CONFIG;

typedef struct lock_t
{
	C_ARRAY char* file;
}LOCK_T;

IO_CONFIG* io_Config(IO_CONFIG *config,FILE *fp);

LOCK_T* lock_Init(LOCK_T *lock,C_ARRAY CHAR_* const filename);

BOOL_ lock_Get(LOCK_T *lock);

void lock_Free(LOCK_T *lock);

void child_Main(IO_CONFIG *config,int listenfd,LOCK_T *child_lock);

int main(int argc,char *argv[]);
