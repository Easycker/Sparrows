#ifndef RESPONDER_H
#define RESPONDER_H

#include "httpd_common.h"

typedef struct mod_t
{
	C_STRING filename;
	C_ARRAY C_STRING *path;
	C_ARRAY C_STRING *dot;
}MOD_T;

typedef struct responder_config
{
	UINT_ port;
	C_ARRAY MOD_T *mods;
	
}RESPONDER_CONFIG;

MOD_T* mod_Create(MOD_T *mod);

int main(int argc,char *argv[]);

#endif