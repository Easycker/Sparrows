#ifndef MOD_CGI_H
#define MOD_CGI_H

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
#include <sys/signal.h>
#include <signal.h>
#include <stdlib.h>
#include "shttpd_type.h"

typedef struct cgi_config
{
	C_STRING root_dir;
	C_STRING index_page;
}CGI_CONFIG;

typedef struct cgi_id
{
	pid_t pid;
	int cgiin;
	int cgiout;
	int state;
	C_ARRAY char* *env_vars;
}CGI_ID;

void put_Env(CGI_ID *connect,CHAR_ const *name,CHAR_ const *value);

/*this is the main function*/

CGI_CONFIG* mod_Config(FILE *fp);

CGI_ID* mod_Prepare(CGI_CONFIG *config,HTTP_REQUEST *request);

int mod_Recv(CGI_ID *connect_id,char *buf,int buf_len);

int mod_Send(CGI_ID *connect_id,char *buf,int buf_len);

void mod_End(CGI_ID *connect_id);

void mod_Unload(CGI_CONFIG *config);

#endif
