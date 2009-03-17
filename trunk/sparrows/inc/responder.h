#ifndef RESPONDER_H
#define RESPONDER_H

#include <cda/c_hash.h>
#include "httpd_common.h"

#define HASH_KEY_ ENCODE_("%S%S")

typedef struct mod_t
{
	C_STRING filename;
}MOD_T;

typedef struct hash_mod
{
	C_STRING key;
	MOD_T *mod;
}HASH_MOD;

typedef struct responder_config
{
	UINT_ port;
	C_ARRAY MOD_T *mods;
	C_HASH mod_table;
}RESPONDER_CONFIG;

RESPONDER_CONFIG* responder_Config(RESPONDER_CONFIG *config,FILE *fp);

int main(int argc,char *argv[]);

#endif