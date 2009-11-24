#ifndef CGI_H
#define CGI_H

#include "shttpd_type.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/fcntl.h>
#include <regex.h>
#include <errno.h>

#define CGI_NAME "cgi"

#define HTTP_OK_ "HTTP/1.1 200 OK\r\n"

#define HTTP_OK_TYPE_ "HTTP/1.1 200 OK\r\nContent-type: %s\r\n\r\n"

#define HTTP_NOT_FOUND_MSG_ "HTTP/1.1 404 NOT FOUND\r\nContent-Type: text/plain\r\n\r\n404 - Not found"

#define NOTHING_ 0
#define CONNECTING_ 1
#define LOCAL_SEND_REMOTE_ 2
#define REMOTE_SEND_LOCAL_ 3

#ifndef SPLICE_F_NONBLOCK
#define SPLICE_F_NONBLOCK 2
#endif

typedef struct cgi_regex
{
	regex_t preg;
	C_STRING exec_path;
	C_STRING parse;
	C_STRING type;
}CGI_REGEX;

typedef struct cgi_config
{
	C_ARRAY CGI_REGEX *regex_list;
	C_STRING index_page;
	int buf_size;
	int new_fd1;
	int new_fd2;
	int close_fd1;
	int close_fd2;
	C_HASH id_list;
	C_DLIST owner_list;
}CGI_CONFIG;

typedef struct cgi_owner
{
	int in_fd;
	int cgi_in;
	int cgi_out;
	int last_op;
	int len;
	char *in_buf;
	char *out_buf;
	int in_buflen;
	int out_buflen;
	pid_t pid;
	BOOL_ in_read;
	BOOL_ remote_read;
	BOOL_ in_write;
	BOOL_ remote_write;
}CGI_OWNER;

typedef struct cgi_id
{
	int fd;
	CGI_OWNER *owner;
}CGI_ID;

UINT_ cgi_Tinyhash(CGI_ID *id);

BOOL_ cgi_Ensure(CGI_ID *lhs,CGI_ID *rhs);

int fd_Setnonblocking(int fd);

/*this is the main function*/
CGI_CONFIG* cgi_Init(IO_CONFIG *io_config,CHAR_ const *arg);

int cgi_Select(CGI_CONFIG *config,HTTP_REQUEST *request,int fd);

int cgi_Work(CGI_CONFIG *config,HTTP_CONNECT *connect);

int cgi_Addport(CGI_CONFIG *config,HTTP_CONNECT *connect,PORT_APPLY *apply);

int cgi_Closeport(CGI_CONFIG *config,HTTP_CONNECT *connect,PORT_APPLY *apply);

int cgi_Close(CGI_CONFIG *config,HTTP_CONNECT *connect);

void cgi_Unload(CGI_CONFIG *config);

#endif
