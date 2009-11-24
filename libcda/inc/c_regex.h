#ifndef C_REGEX_H
#define C_REGEX_H

#include "c_mem.h"
#include "c_array.h"
#include "c_string.h"
#include "c_list.h"
#include <stdint.h>
#include <wchar.h>

#define OK_ 0x00000000
#define EQUAL_ 0x0000000f
#define UNEQUAL_ 0x000000f0
#define INC_ 0x00000f00
#define FINISH_ 0xf0000000

#define SET_ 0x0000000f

typedef struct regex_element
{
	uint32_t type;
	C_STRING set;
	C_LIST next;
}REGEX_ELM;

typedef struct regex_prefix
{
	REGEX_ELM *entry;
	C_POOL elm_pool;
	C_LIST set_list;
}REGEX_PREFIX;

typedef struct regex_result
{
	size_t start;
	size_t end;
}REGEX_RESULT;

REGEX_PREFIX* regex_Prepare(REGEX_PREFIX *prefix,char const *str,size_t length);

REGEX_RESULT* regex_Exec(REGEX_RESULT *result,REGEX_PREFIX const *prefix,char const *target,size_t length);

#endif
