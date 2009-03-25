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
	C_ARRAY MIME *mime_list;
}STATIC_CONFIG;

CHAR_* dot_Check(const CHAR_ *filename);

size_t file_Read(C_ARRAY char **array,FILE *fp);

/*this is the main function*/
STATIC_CONFIG* mod_Config(FILE *fp);

BOOL_ mod_Main(HTTP_REQUEST *request,STATIC_CONFIG *config,int fd);

void mod_Unload(STATIC_CONFIG *config);

#endif