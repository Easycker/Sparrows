#ifndef MOD_CGI_H
#define MOD_CGI_H

#include <cda/c_define.h>
#include <cda/c_string.h>
#include <cda/c_hash.h>
#include <cda/c_dchain.h>
#include <xmlo/xmlo.h>
#include <stdio.h>
#include <stdlib.h>
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
#include "shttpd_type.h"

#define HTTP_OK_ ENCODE_("HTTP/1.1 200 OK\r\n")

#define HTTP_NOT_FOUND_ ENCODE_("HTTP/1.1 404 NOT FOUND\r\n")

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
	C_STRING exec_path;
}REGEX_NODE;

typedef struct cgi_config
{
	C_ARRAY REGEX_NODE *regex_list;
	C_STRING index_page;
	int buf_size;
	int new_fd1;
	int new_fd2;
	int close_fd1;
	int close_fd2;
	C_HASH id_list;
	C_DCHAIN owner_list;
}CGI_CONFIG;

typedef struct cgi_owner
{
	int in_fd;
	int cgi_in;
	int cgi_out;
	int last_op;
	int len;
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
CGI_CONFIG* mod_Init(CHAR_ const *arg);

int mod_Select(CGI_CONFIG *config,HTTP_REQUEST *request,int fd);

int mod_Work(CGI_CONFIG *config,HTTP_CONNECT *connect);

int mod_Addport(CGI_CONFIG *config,HTTP_CONNECT *connect,PORT_APPLY *apply);

int mod_Closeport(CGI_CONFIG *config,HTTP_CONNECT *connect,PORT_APPLY *apply);

int mod_Close(CGI_CONFIG *config,HTTP_CONNECT *connect);

void mod_Unload(CGI_CONFIG *config);

#endif
