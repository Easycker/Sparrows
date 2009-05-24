#ifndef MOD_REWRITE_H
#define MOD_REWRITE_H

#include <cda/c_define.h>
#include <cda/c_string.h>
#include <cda/c_hash.h>
#include <xmlo/xmlo.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <regex.h>
#include <errno.h>
#include "shttpd_type.h"

typedef struct regex_node
{
	regex_t preg;
	C_STRING root_dir;
}REGEX_NODE;

typedef struct rewrite_config
{
	C_STRING index_page;
	C_ARRAY REGEX_NODE *regex_list;
	C_ARRAY MIME *mime_list;
	int buf_size;
	C_HASH id_list;
}REWRITE_CONFIG;

typedef struct rewrite_id
{
	CHAR_ const *root_dir;
	int connect_fd;
	int file_fd;
	int buf_len;
	char *buf;
}REWRITE_ID;

CHAR_* dot_Check( CHAR_ *const filename);

UINT_ rewrite_Tinyhash(REWRITE_ID *id);

BOOL_ rewrite_Ensure(REWRITE_ID *lhs,REWRITE_ID *rhs);

/*this is the main function*/
REWRITE_CONFIG* mod_Init(CHAR_ const *arg);

int mod_Select(REWRITE_CONFIG *config,HTTP_REQUEST *request,int fd);

int mod_Work(REWRITE_CONFIG *config,HTTP_CONNECT *connect);

int mod_Addport(REWRITE_CONFIG *config,HTTP_CONNECT *connect,PORT_APPLY *apply);

void mod_Unload(REWRITE_CONFIG *config);

#endif
