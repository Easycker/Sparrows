#ifndef MOD_PROXY_H
#define MOD_PROXY_H

#include <cda/c_define.h>
#include <cda/c_string.h>
#include <cda/c_hash.h>
#include <cda/c_dchain.h>
#include <xmlo/xmlo.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/fcntl.h>
#include <regex.h>
#include <errno.h>
#include "shttpd_type.h"

#define NOTHING_ 0
#define CONNECTING_ 1
#define LOCAL_SEND_REMOTE_ 2
#define REMOTE_SEND_LOCAL_ 3

#ifndef SPLICE_F_NONBLOCK
#define SPLICE_F_NONBLOCK 2
#endif

typedef struct regex_node
{
	regex_t preg;
	struct sockaddr_in addr;
}REGEX_NODE;

typedef struct proxy_config
{
	C_ARRAY REGEX_NODE *regex_list;
	int buf_size;
	int new_fd;
	int close_fd;
	C_HASH id_list;
	C_DCHAIN owner_list;
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
PROXY_CONFIG* mod_Init(CHAR_ const *arg);

int mod_Select(PROXY_CONFIG *config,HTTP_REQUEST *request,int fd);

int mod_Work(PROXY_CONFIG *config,HTTP_CONNECT *connect);

int mod_Addport(PROXY_CONFIG *config,HTTP_CONNECT *connect,PORT_APPLY *apply);

int mod_Closeport(PROXY_CONFIG *config,HTTP_CONNECT *connect,PORT_APPLY *apply);

int mod_Close(PROXY_CONFIG *config,HTTP_CONNECT *connect);

void mod_Unload(PROXY_CONFIG *config);

#endif
