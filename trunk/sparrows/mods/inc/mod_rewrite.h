#ifndef MOD_REWRITE_H
#define MOD_REWRITE_H

#include <cda/c_define.h>
#include <cda/c_string.h>
#include <cda/c_hash.h>
#include <xmlo/xmlo.h>
#include <stdio.h>
#include <unistd.h>
#include <regex.h>
#include <errno.h>
#include "shttpd_type.h"

#define MAX_REGEX_ 32

typedef struct regex_node
{
	regex_t preg;
	C_STRING replace;
}REGEX_NODE;

typedef struct rewrite_config
{
	C_ARRAY REGEX_NODE *regex_list;
}REWRITE_CONFIG;

/*this is the main function*/
REWRITE_CONFIG* mod_Init(CHAR_ const *arg);

int mod_Select(REWRITE_CONFIG *config,HTTP_REQUEST *request,int fd);

int mod_Work(REWRITE_CONFIG *config,HTTP_CONNECT *connect);

int mod_Addport(REWRITE_CONFIG *config,HTTP_CONNECT *connect,PORT_APPLY *apply);

void mod_Unload(REWRITE_CONFIG *config);

#endif
