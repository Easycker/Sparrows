#ifndef MOD_PROXY_H
#define MOD_PROXY_H

#include <cda/c_define.h>
#include <cda/c_string.h>
#include <cda/c_hash.h>
#include <xmlo/xmlo.h>
#include <stdio.h>
#include <wchar.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <errno.h>
#include "shttpd_type.h"

typedef struct proxy_config
{
}PROXY_CONFIG;

typedef struct PROXY_id
{
}PROXY_ID;

UINT_ porxy_Tinyhash(PROXY_ID *id);

BOOL_ proxy_Ensure(PROXY_ID *lhs,PROXY_ID *rhs);

/*this is the main function*/
PROXY_CONFIG* mod_Init(CHAR_ const *arg);

int mod_Select(PROXY_CONFIG *config,HTTP_REQUEST *request,int fd);

int mod_Work(PROXY_CONFIG *config,HTTP_CONNECT *connect);

int mod_Addport(PROXY_CONFIG *config,HTTP_CONNECT *connect,PORT_APPLY *apply);

void mod_Unload(PROXY_CONFIG *config);

#endif
