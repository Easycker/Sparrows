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
#include <netinet/tcp.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <regex.h>
#include <sys/sendfile.h>
#include <errno.h>
#include "shttpd_type.h"

#define HTTP_OK_ ENCODE_("HTTP/1.1 200 OK\r\n")

#define HTTP_NOT_FOUND_ ENCODE_("HTTP/1.1 404 NOT FOUND\r\n")

#define HTTP_TYPE_ ENCODE_("Content-Type:%S\r\n")

#define HTTP_LENGTH_ ENCODE_("Content-Length:%lld\r\n")

#define NEW_LINE_ ENCODE_("\r\n")

typedef struct mime
{
	C_STRING mime_dot;
	C_STRING mime_type;
}MIME;

typedef struct regex_node
{
	regex_t preg;
	C_STRING root_dir;
}REGEX_NODE;

typedef struct static_config
{
	C_STRING index_page;
	C_ARRAY REGEX_NODE *regex_list;
	C_ARRAY MIME *mime_list;
	int buf_size;
	C_HASH id_list;
}STATIC_CONFIG;

typedef struct static_id
{
	CHAR_ const *root_dir;
	int connect_fd;
	int file_fd;
	int buf_len;
	int send_buf;
	off_t send_off;
	off_t file_size;
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

void mod_Close(STATIC_CONFIG *config,HTTP_CONNECT *connect);

void mod_Unload(STATIC_CONFIG *config);

#endif
