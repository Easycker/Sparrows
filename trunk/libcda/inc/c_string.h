#ifndef C_STRING_H
#define C_STRING_H

#include <stdio.h>
#include <stdlib.h>
#include "c_array.h"
#include "c_define.h"

#include <wchar.h>
#include <locale.h>

#define CHAR_ char
#define WORD_SIZE_ (sizeof(wchar_t)+1)

#if 1
#define ENCODE_(string) string
#define CHAR_ char
#define STRLEN_ strlen
#define STRCMP_ strcmp
#define STRCPY_ strcpy
#define PRINTF_ printf
#define FPRINTF_ fprintf
#define SNPRINTF_ snprintf
#define GETC_ getc
#define FGETC_ fgetc
#define FPUTC_ fputc

#define STRTIME_ strftime

#define ISDIGIT_ isdigit
#define STRTOL_ strtol
#define STRTOI_ strtoi
#define STRTOUL_ strtoul
#define ATOL_ atol

#endif

#define string_Drop array_Drop

/*typedef CHAR_* C_STRING;*/
typedef char* C_STRING;
typedef wchar_t* C_WSTRING;

typedef struct pre_kmp
{
	char *key_string;
	size_t *prefix_array;
	size_t key_length;
}KMP_KEY;

#define string_Create() (array_Create(sizeof(CHAR_)))

C_STRING string_Create_Ex(char const *tag_string);

KMP_KEY* string_Kprepare(KMP_KEY *tag_key,char const *key_string);

CHAR_* string_Knsearch(char const *tag_string,KMP_KEY const *key,size_t n);

void string_Kfree(KMP_KEY *key);

C_STRING string_Set(C_STRING *tag_array_p,char const *tag_string);

C_STRING string_Append(C_STRING *tag_string,char *s_char);

C_STRING string_Link(C_STRING *left_string,C_STRING right_string);

char* string_Widetoansi(char** tag_string,const wchar_t *wstring);

wchar_t* string_Ansitowide(wchar_t **tag_string,const char *string);

#endif
