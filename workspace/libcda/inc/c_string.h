#ifndef C_STRING_H
#define C_STRING_H

#include <stdio.h>
#include "c_array.h"
#include "c_define.h"

#ifndef WITHOUT_WIDECHAR_SUPPORT_
#include <wchar.h>
#include <locale.h>

#define SET_LOCALE_(locale) (setlocale(LC_ALL,locale))
#define WORD_SIZE_ (sizeof(wchar_t)+1)

#define ENCODE_(string) L##string
#define CHAR_ wchar_t
#define STRLEN_ wcslen
#define STRCMP_ wcscmp
#define STRCPY_ wcscpy
#define PRINTF_ wprintf
#define FPRINTF_ fwprintf
#define SNPRINTF_ swprintf
#define GETC_ getwc
#define FGETC_ fgetwc
#define FPUTC_ fputwc

#define STRTIME_ wcsftime

#define ISDIGIT_ iswdigit
#define STRTOL_ wcstol
#define STRTOI_ wcstoi
#define STRTOUL_ wcstoul
#define ATOL_ wcatol

/*convert widechar to ansi char*/

#else
#define ENCODE_(string) string
#define CHAR_ char
#define STRLEN_ strlen
#define STRCMP_ strcmp
#define STRCPY_ strcpy
#define PRINTF_ printf
#define FPRINTF_ printf
#define SPRINTF_ sprintf

#endif

#define string_Drop array_Drop

typedef CHAR_* C_STRING;

typedef struct pre_kmp
{
	CHAR_ *key_string;
	size_t *prefix_array;
	size_t key_length;
}KMP_KEY;

#define string_Create() (array_Create(sizeof(CHAR_)))

C_STRING string_Create_Ex(CHAR_ const *tag_string);

KMP_KEY* string_Kprepare(KMP_KEY *tag_key,CHAR_ const *key_string);

CHAR_* string_Knsearch(CHAR_ const *tag_string,KMP_KEY const *key,size_t n);

void string_Kfree(KMP_KEY *key);

C_STRING string_Set(C_STRING *tag_array_p,CHAR_ const *tag_string);

char* string_Widetoansi(char** tag_string,const wchar_t *wstring);

wchar_t* string_Ansitowide(wchar_t **tag_string,const char *string);

#endif
