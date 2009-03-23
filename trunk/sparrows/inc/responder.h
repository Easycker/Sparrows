#ifndef RESPONDER_H
#define RESPONDER_H

#include <cda/c_string.h>
#include <cda/c_define.h>
#include <cda/c_hash.h>
#include <xmlo/xmlo.h>
#include <dlfcn.h>
#include <stdint.h>
#include <unistd.h>
#include "httpd_common.h"

#define HASH_KEY_ ENCODE_("%S%S")

typedef struct mod_t
{
	C_STRING key;
	void (*mod_Main)();
}MOD_T;

/*a mod_Main: BOOL_ mod_Main()*/

/*
typedef struct hash_mod
{
	C_STRING key;
	MOD_T *mod;
}HASH_MOD;
*/

typedef struct responder_config
{
	UINT_ port;
	C_HASH mod_table;
}RESPONDER_CONFIG;

UINT_ elf_Tinyhash(MOD_T *mod);

RESPONDER_CONFIG* responder_Config(RESPONDER_CONFIG *config,FILE *fp);

void config_Drop(RESPONDER_CONFIG *config);

BOOL_ mod_Exec(RESPONDER_CONFIG *config,HTTP_REQUEST *request,int fd);

/*int main(int argc,char *argv[]);*/

#endif