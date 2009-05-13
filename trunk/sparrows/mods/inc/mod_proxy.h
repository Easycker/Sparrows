#ifndef MOD_PROXY_H
#define MOD_PROXY_H

#include <cda/c_define.h>
#include <cda/c_string.h>
#include <cda/c_hash.h>
#include <cda/c_chain.h>
#include <xmlo/xmlo.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <regex.h>
#include <errno.h>
#include "shttpd_type.h"

#define STATE_CONNECT_ 0
#define STATE_SEND_ 1
#define STATE_RECV_ 2

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
	C_HASH id_list;
	C_CHAIN owner_list;
}PROXY_CONFIG;

typedef struct proxy_owner
{
	int in_fd;
	int remote_fd;
	int state;
	char *buf;
	int buf_len;
	BOOL_ in_ready;
	BOOL_ remote_ready;
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

void mod_Unload(PROXY_CONFIG *config);

#endif
