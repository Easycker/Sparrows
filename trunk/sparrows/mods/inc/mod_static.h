#ifndef MOD_STATIC_H
#define MOD_STATIC_H

#include <cda/c_define.h>
#include <cda/c_string.h>
#include <xmlo/xmlo.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
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
	C_ARRAY MIME *mime_list;
}STATIC_CONFIG;

typedef struct static_id
{
	FILE *fp;
	C_STRING mime;
	int state;
}STATIC_ID;

CHAR_* dot_Check( CHAR_ *const filename);

/*this is the main function*/
STATIC_CONFIG* mod_Config(FILE *fp);

STATIC_ID* mod_Prepare(STATIC_CONFIG *config,HTTP_REQUEST *request);

int mod_Recv(STATIC_ID *connect_id,char *buf,int buf_len);

int mod_Send(STATIC_ID *connect_id,char *buf,int buf_len);

void mod_End(STATIC_ID *connect_id);

void mod_Unload(STATIC_CONFIG *config);

#endif
