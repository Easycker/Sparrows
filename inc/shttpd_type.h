#ifndef SHTTPD_TYPE_H
#define SHTTPD_TYPE_H

#include "config.h"
#include "c_hash.h"
#include "c_mem.h"
#include "c_string.h"
#include "c_list.h"
#include "c_xml.h"
#include "c_bhtree.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>

#ifdef DEBUG_ENABLE
#define DEBUG_
#define DEBUG2_
#endif

#ifdef DEBUG_
#define ERROR_OUT_ FPRINTF_
#else
#define ERROR_OUT_
#endif
#ifdef DEBUG2_
#define ERROR_OUT2_ FPRINTF_
#define ERROR_PRINT_ do{perror("");ERROR_OUT2_(stderr,"ERROR ON file:%s,function:%s,line:%d\n",__FILE__,__FUNCTION__,__LINE__);}while(0)
#else
#define ERROR_OUT2_
#define ERROR_PRINT_
#endif

#define ERROR_EXIT_ do{ERROR_PRINT_;goto fail_return;}while(0)

#define SERVER_NAME_ "sparrows web server"
#define SERVER_VERSION_ "0.1"

#define HASH_SPACE_ 0x0000ffff

#define SELECT_INPUT_ 0xf0000000
#define SELECT_OUTPUT_ 0x0f000000
#define SELECT_GOON_ 0x00f00000
#define SELECT_BREAK_ 0x000f0000
#define SELECT_NEWPORT_ 0x0000f000

#define WORK_CLOSE_ 0xf0000000
#define WORK_NEWPORT_ 0x0f000000
#define WORK_GOON_ 0x00f00000
#define WORK_INPUT_ 0x000f0000
#define WORK_OUTPUT_ 0x0000f000
#define WORK_UNLOAD_ 0x00000f00
#define WORK_KEEP_ 0x000000f0
#define WORK_CLOSEPORT_ 0x0000000f

#include <regex.h>

typedef enum request_type
{
	GET=0,
	POST=1
}REQUEST_TYPE;

typedef struct http_request
{
	char *recv_data;
	int recv_len;

	REQUEST_TYPE type;
	C_STRING path;

	C_STRING host;
	BOOL_ alive;
	
	/*common cgi var*/
	C_STRING accept;
	C_STRING accept_charset;
	C_STRING accept_encoding;
	C_STRING accept_language;
	C_STRING cookies;
	C_STRING referer;
	C_STRING user_agent;
	C_STRING query_string;
	C_STRING remote_addr;

	/*maybe post data?*/
	C_STRING content_type;
	C_STRING content_length;
}HTTP_REQUEST;

typedef struct mod_t
{
	void *share;
	void *mod_lib;
	void* (*mod_Init)();
	int (*mod_Select)();
	int (*mod_Work)();
	int (*mod_Addport)();
	int (*mod_Closeport)();
	int (*mod_Close)();
	void (*mod_Unload)();
}MOD_T;

typedef struct modmanage_config
{
	C_ARRAY MOD_T* mod_table;
	int buf_len;
}MODMANAGE_CONFIG;

typedef struct http_connect
{
	int fd;
	int event;
	time_t last_op;
	BOOL_ op_done;
	MOD_T *mod;
}HTTP_CONNECT;

typedef struct port_apply
{
	int fd;
}PORT_APPLY;

typedef struct host_type
{
	regex_t preg;
	MODMANAGE_CONFIG mod_config;
}HOST_TYPE;

typedef struct io_config
{
	struct sockaddr_in addr;
	int http_port;
	int io_buf;
	int pool_length;
	int pool_timeout;
	int keep_alive;
	int timeout;
	int max_head;
	C_ARRAY HOST_TYPE *host_list;
	C_STRING config_path;
}IO_CONFIG;

#endif
