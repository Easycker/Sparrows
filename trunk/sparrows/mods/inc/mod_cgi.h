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

typedef struct exec
{
	C_STRING exec_dot;
	C_STRING exec_type;
}EXEC;

typedef struct cgi_config
{
	C_STRING root_dir;
	C_STRING index_page;
	C_ARRAY EXEC* exec_list;
}CGI_CONFIG;

CHAR_* dot_Check(CHAR_ *const filename);

CGI_CONFIG* mod_Config(FILE *fp);

BOOL_ mod_Main(HTTP_REQUEST *request,CGI_CONFIG *config,int fd);

void mod_Unload(CGI_CONFIG *config);
