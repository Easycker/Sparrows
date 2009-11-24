#ifndef REWRITE_H
#define REWRITE_H

#include "shttpd_type.h"
#include <stdio.h>
#include <unistd.h>
#include <regex.h>
#include <errno.h>

#define REWRITE_NAME "rewrite"

#define MAX_REGEX_ 32

typedef struct rewrite_regex
{
	regex_t preg;
	C_STRING replace;
}REWRITE_REGEX;

typedef struct rewrite_config
{
	C_ARRAY REWRITE_REGEX *regex_list;
}REWRITE_CONFIG;

/*this is the main function*/
REWRITE_CONFIG* rewrite_Init(IO_CONFIG *io_config,CHAR_ const *arg);

int rewrite_Select(REWRITE_CONFIG *config,HTTP_REQUEST *request,int fd);

int rewrite_Work(REWRITE_CONFIG *config,HTTP_CONNECT *connect);

int rewrite_Addport(REWRITE_CONFIG *config,HTTP_CONNECT *connect,PORT_APPLY *apply);

int rewrite_Closeport(REWRITE_CONFIG *config,HTTP_CONNECT *connect,PORT_APPLY *apply);

int rewrite_Close(REWRITE_CONFIG *config,HTTP_CONNECT *connect);

void rewrite_Unload(REWRITE_CONFIG *config);

#endif
