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

#define HOWTO_ ENCODE_("example:./server <filename>\n")

typedef struct io_config
{
	UINT_ max_process;
	UINT_ stable_process;
	UINT_ http_port;
	C_STRING cache_adr;
	UINT_ cache_port;
	int resv_buf;
}IO_CONFIG;

typedef enum io_signal
{
	READY=0,
	BUSY=1,
	START=2
}IO_SIGNAL;

IO_CONFIG* io_Config(IO_CONFIG *config,FILE *fp);

void child_Main(IO_CONFIG *config,int listenfd);

int main(int argc,char *argv[]);
