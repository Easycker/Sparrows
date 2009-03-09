#ifndef SHTTPD_DEF
#define SHTTPD_DEF

#include <unistd.h>
#include <cda/c_define.h>

typedef struct request_t
{
	pid_t pid;
	CHAR_ path[256];
	CHAR_ post_data[256];
}REQUEST_TYPE;

#endif