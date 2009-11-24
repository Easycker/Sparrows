#ifndef STATIC_H
#define STATIC_H

#include "shttpd_type.h"
#include <stdio.h>
#include <stdlib.h>
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
#ifdef SENDFILE_ENABLE
#include <sys/sendfile.h>
#endif
#include <errno.h>

#define STATIC_NAME "static"

#define MAX_LINE_ 1024

#define HTTP_HEADER_ "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %ld\r\n\r\n"

#define HTTP_HEADER_KEEPALIVE_ "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %ld\r\nKeep-Alive: timeout=%d,max=%d\r\n\r\n"

#define HTTP_NOT_FOUND_ "HTTP/1.1 404 NOT FOUND\r\nContent-Type: text/html\r\n\r\n"

#define HTTP_NOT_FOUND_MSG_ "HTTP/1.1 404 NOT FOUND\r\nContent-Type: text/plain\r\n\r\n404 - Not found"

#define NEW_LINE_ ENCODE_("\r\n")

typedef struct mime
{
	C_STRING mime_dot;
	C_STRING mime_type;
}MIME;

typedef struct static_regex
{
	regex_t preg;
	C_STRING root_dir;
}STATIC_REGEX;

typedef struct static_config
{
	C_STRING index_page;
	C_STRING error_page;
	C_ARRAY STATIC_REGEX *regex_list;
	C_ARRAY MIME *mime_list;
	int buf_size;
	C_HASH id_list;
}STATIC_CONFIG;

typedef struct static_id
{
	CHAR_ const *root_dir;
	BOOL_ alive;
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
STATIC_CONFIG* static_Init(IO_CONFIG *io_config,CHAR_ const *arg);

int static_Select(STATIC_CONFIG *config,HTTP_REQUEST *request,int fd);

int static_Work(STATIC_CONFIG *config,HTTP_CONNECT *connect);

int static_Addport(STATIC_CONFIG *config,HTTP_CONNECT *connect,PORT_APPLY *apply);

int static_Closeport(STATIC_CONFIG *config,HTTP_CONNECT *connect,PORT_APPLY *apply);

int static_Close(STATIC_CONFIG *config,HTTP_CONNECT *connect);

void static_Unload(STATIC_CONFIG *config);

#endif
