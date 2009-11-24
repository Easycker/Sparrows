#include "c_string.h"

C_STRING string_Create_Ex(char const *tag_string)
{
	C_STRING res_string;
	
	res_string=array_Create(sizeof(char));
	if(string_Set(&res_string,tag_string)==NULL)return NULL;
	
	return res_string;
};

KMP_KEY* string_Kprepare(KMP_KEY *tag_key,char const *key_string)
{
	size_t i;
	size_t j;
	
	tag_key->key_string=NULL;
	tag_key->prefix_array=NULL;
	tag_key->key_length=strlen(key_string);
	tag_key->key_string=malloc(sizeof(char)*(tag_key->key_length+1));
	if(tag_key->key_string==NULL)goto fail_return;
	memcpy(tag_key->key_string,key_string,(tag_key->key_length+1)*sizeof(char));

	tag_key->prefix_array=(size_t*)malloc(sizeof(size_t)*(tag_key->key_length));
	if(tag_key->prefix_array==NULL)goto fail_return;

	/*for(i=0;i<(tag_key->key_length);++i)tag_key->prefix_array[i]=0;*/
	tag_key->prefix_array[0]=0;
	i=0;
	for(j=1;j<(tag_key->key_length);++j)
	{
		while(i>0&&(key_string[i]!=key_string[j]))
		{
			i=tag_key->prefix_array[i-1];
		}
		if(key_string[i]==key_string[j])++i;
		tag_key->prefix_array[j]=i;
	};

	return tag_key;

	fail_return:
	if(tag_key->key_string!=NULL)free(tag_key->key_string);
	if(tag_key->prefix_array!=NULL)free(tag_key->prefix_array);
	return NULL;
};

char* string_Knsearch(char const *tag_string,KMP_KEY const *key,size_t n)
{
	size_t i;
	size_t j;
	size_t k;

	for(i=k=j=0;(j<(strlen(tag_string)))&&(n==0?1:(k<n));++j,++k)
	{
		while(i>0&&((key->key_string)[i]!=tag_string[j]))i=(key->prefix_array)[i-1];
		if((key->key_string)[i]==tag_string[j])++i;

		if(i>=(key->key_length))return (char* const)&(tag_string[j+1-key->key_length]);
	};
	return NULL;
};

void string_Kfree(KMP_KEY *key)
{
	free(key->key_string);
	free(key->prefix_array);
};

C_STRING string_Set(C_STRING *tag_array_p,char const *tag_string)
{
	/*char const *op;*/
	C_STRING tag_array;
	size_t len;

	tag_array=*tag_array_p;
	len=strlen(tag_string)+1;
	if(len>=array_Length(tag_array))array_Resize(tag_array_p,len);
	memcpy(*tag_array_p,tag_string,len);
	array_Length(*tag_array_p)=len;
	
	return tag_array;
};

C_STRING string_Append(C_STRING *tag_string,char *s_char)
{
	char end_char;

	end_char=ENCODE_('\0');
	if(array_Length(*tag_string)>0&&((*tag_string)[array_Length(*tag_string)-1]==ENCODE_('\0')))
	{
		array_Remove(tag_string,&((*tag_string)[array_Length(*tag_string)-1]));
	};
	array_Append(tag_string,s_char);
	array_Append(tag_string,&end_char);
	return *tag_string;
};

C_STRING string_Link(C_STRING *left_string,C_STRING right_string)
{
	char *op;

	for(op=right_string;*op!=ENCODE_('\0');++op)string_Append(left_string,op);
	return *left_string;
};

char* string_Widetoansi(char** tag_string,wchar_t const *wstring)
{
	wchar_t const *s_wchar;
	char char_cache[WORD_SIZE_];
	char *op;
	size_t to_end;
	
	array_Resize(tag_string,0);
	for(s_wchar=wstring;*s_wchar!=L'\0';++s_wchar)
	{
		to_end=wctomb(char_cache,*s_wchar);
		char_cache[to_end]='\0';
		for(op=char_cache;*op!='\0';++op)array_Append(tag_string,op);
	};
	*op='\0';
	array_Append(tag_string,op);
	return *tag_string;
};

wchar_t* string_Ansitowide(wchar_t **tag_wstring,char const *string)
{
	char const *s_char;
	wchar_t s_wchar=L'\0';
	
	array_Resize(tag_wstring,0);
	for(s_char=string;*s_char!='\0';s_char+=mbrtowc(&s_wchar,s_char,WORD_SIZE_,NULL))*tag_wstring=(s_wchar==L'\0'?*tag_wstring:array_Append(tag_wstring,&s_wchar));
	array_Append(tag_wstring,&s_wchar);
	s_wchar=L'\0';
	array_Append(tag_wstring,&s_wchar);
	
	return *tag_wstring;
};
