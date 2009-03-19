#ifndef RESPONDER_H
#define RESPONDER_H

#include <cda/c_string.h>
#include <cda/c_define.h>
#include <cda/c_hash.h>
#include <xmlo/xmlo.h>
#include <dlfcn.h>
#include <stdint.h>
#include "httpd_common.h"

#define HASH_KEY_ ENCODE_("%S%S")

#define HOWTO_ ENCODE_("howto?\n")

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
	C_ARRAY MOD_T *mods;
	C_HASH mod_table;
}RESPONDER_CONFIG;

unsigned char elf_Tinyhash(MOD_T *mod);

RESPONDER_CONFIG* responder_Config(RESPONDER_CONFIG *config,FILE *fp);

int main(int argc,char *argv[]);

#endif