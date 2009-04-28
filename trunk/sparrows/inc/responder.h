#ifndef RESPONDER_H
#define RESPONDER_H

#include <cda/c_string.h>
#include <cda/c_define.h>
#include <cda/c_hash.h>
#include <xmlo/xmlo.h>
#include <dlfcn.h>
#include <stdint.h>
#include <unistd.h>
#include "request.h"

#define HASH_KEY_ ENCODE_("%S%S")

typedef struct mod_t
{
	C_STRING key;
	void *config;
	void *mod_lib;
	void* (*mod_Config)();
	void* (*mod_Prepare)();
	int (*mod_Recv)();
	int (*mod_Send)();
	void (*mod_End)();
	void (*mod_Unload)();
}MOD_T;

typedef struct responder_config
{
	C_HASH mod_table;
	int buf_len;
}RESPONDER_CONFIG;

UINT_ elf_Tinyhash(MOD_T *mod);

RESPONDER_CONFIG* responder_Config(RESPONDER_CONFIG *config,FILE *fp);

void mod_Drop(MOD_T *mod);

void responder_Drop(RESPONDER_CONFIG *config);

MOD_T* mod_Find(RESPONDER_CONFIG *config,HTTP_REQUEST *request);

/*int main(int argc,char *argv[]);*/

#endif
