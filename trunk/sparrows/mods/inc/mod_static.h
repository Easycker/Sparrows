#ifndef MOD_STATIC_H
#define MOD_STATIC_H

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

typedef struct mime
{
	C_STRING mime_dot;
	C_STRING mime_type;
}MIME;

typedef struct static_config
{
	C_STRING root_dir;
	C_STRING index_page;
	regex_t preg;
	C_ARRAY MIME *mime_list;
	int buf_size;
	C_HASH id_list;
}STATIC_CONFIG;

typedef struct static_id
{
	int connect_fd;
	int file_fd;
	int buf_len;
	char *buf;
}STATIC_ID;

CHAR_* dot_Check( CHAR_ *const filename);

UINT_ static_Tinyhash(STATIC_ID *id);

BOOL_ static_Ensure(STATIC_ID *lhs,STATIC_ID *rhs);

/*this is the main function*/
STATIC_CONFIG* mod_Init(CHAR_ const *arg);

int mod_Select(STATIC_CONFIG *config,HTTP_REQUEST *request,int fd);

int mod_Work(STATIC_CONFIG *config,HTTP_CONNECT *connect);

int mod_Addport(STATIC_CONFIG *config,HTTP_CONNECT *connect,PORT_APPLY *apply);

void mod_Unload(STATIC_CONFIG *config);

#endif
