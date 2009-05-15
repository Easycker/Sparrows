#ifndef SHTTPD_TYPE_H
#define SHTTPD_TYPE_H

/*#define DEBUG_*/
#define DEBUG2_
#ifdef DEBUG_
#define ERROR_OUT_ FPRINTF_
#else
#define ERROR_OUT_
#endif
#ifdef DEBUG2_
#define ERROR_OUT2_ FPRINTF_
#else
#define ERROR_OUT2
#endif

#define HASH_SPACE_ 0x0000ffff

#define HTTP_OK_ ENCODE_("HTTP/1.1 200 OK\r\n")

#define HTTP_NOT_FOUND_ ENCODE_("HTTP/1.1 404 NOT FOUND\r\n")

#define HTTP_TYPE_ ENCODE_("Content-Type:%S\r\n")

#define HTTP_LENGTH_ ENCODE_("Content-Length:%lld\r\n")

#define NEW_LINE_ ENCODE_("\r\n")

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

#include <regex.h>

typedef enum request_type
{
	GET=0,
	POST=1
}REQUEST_TYPE;

typedef struct http_request
{
	C_STRING recv_data;

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
	int (*mod_Unload)();
}MOD_T;

typedef struct modmanage_config
{
	C_ARRAY MOD_T* mod_table;
	int buf_len;
}MODMANAGE_CONFIG;

/*
typedef enum http_state
{
	LISTEN=0,
	WORK=1
}HTTP_STATE;
*/

typedef struct http_connect
{
	int fd;
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

#endif
