#ifndef MODMANAGE_H
#define MODMANAGE_H

#include <cda/c_string.h>
#include <cda/c_define.h>
#include <cda/c_hash.h>
#include <xmlo/xmlo.h>
#include <dlfcn.h>
#include <stdint.h>
#include <unistd.h>
#include "request.h"
#include "shttpd_type.h"

MODMANAGE_CONFIG* modmanage_Init(MODMANAGE_CONFIG *config,FILE *fp);

void modmanage_Drop(MODMANAGE_CONFIG *config);

#endif
