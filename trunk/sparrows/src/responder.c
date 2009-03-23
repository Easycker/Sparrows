#include "responder.h"

UINT_ elf_Tinyhash(MOD_T *mod)
{
	C_STRING string;
	uint32_t hash=0;
	uint32_t x=0 ;
	CHAR_ *op;
	size_t len;

	string=mod->key;
	len=(array_Length(string)+1)*sizeof(CHAR_);
	op=string;
	while(*op!=ENCODE_('\0'))
	{
		hash=(hash<<4)+(*op++);
		if((x=hash&0xF0000000L)!=0)
		{
			hash^= (x>>24);
			hash&=~x;
		};
	};
	return (hash&255);
	/*255=0x000000ff*/
};

BOOL_ ensure_Mod(MOD_T *lhs,MOD_T *rhs)
{
	PRINTF_(ENCODE_("COMPING LHS IS %S,RHS IS %S\n"),lhs->key,rhs->key);
	return !(STRCMP_(lhs->key,rhs->key));
};

RESPONDER_CONFIG* responder_Config(RESPONDER_CONFIG *config,FILE *fp)
{
	XML_DOC doc;
	XML_NODE *config_root;
	XML_NODE *node;
	C_STRING cache;
	C_ARRAY char *cache_ansi;
	C_ARRAY C_STRING *types;
	UINT_ i;
	UINT_ j;
	UINT_ k;
	MOD_T mod;
	void *share_mod;
	C_STRING tmp;

	mod.key=string_Create();
	cache=string_Create();
	cache_ansi=array_Create(sizeof(char));
	xml_Open(&doc,fp,STORE_ALL);
	config_root=xml_Nodebyname(ENCODE_("responder"),doc.root);
	xml_Storedata(&cache,xml_Nodebyname(ENCODE_("port"),config_root),&doc);
	config->port=STRTOUL_(cache,NULL,0);
	hash_Create(&(config->mod_table),&elf_Tinyhash,&ensure_Mod,sizeof(MOD_T),255);
	/*read the mods*/
	for(i=0;i<xml_Nodechilds(xml_Nodebyname(ENCODE_("mods"),config_root));++i)
	{
		/*clean up the mod_t*/
		mod.mod_Main=NULL;
		types=array_Create_Ex(sizeof(C_STRING),&string_Drop);
		/*load the module*/

		xml_Parmbyname(&cache,ENCODE_("path"),xml_Nodebyindex(i,xml_Nodebyname(ENCODE_("mods"),config_root)));

		array_Remove(&cache,&cache[0]);
		array_Remove(&cache,&cache[STRLEN_(cache)-1]);
		string_Widetoansi(&cache_ansi,cache);
		PRINTF_(ENCODE_("WILL LOAD MOD:%S\n"),cache);

		share_mod=dlopen(cache_ansi,RTLD_LAZY);
		mod.mod_Main=dlsym(share_mod, "mod_Main");

		node=xml_Nodebyname(ENCODE_("type"),xml_Nodebyindex(i,xml_Nodebyname(ENCODE_("mods"),config_root)));
		for(j=0;j<xml_Nodechilds(node);++j)
		{
			tmp=string_Create_Ex(xml_Parmbyname(&cache,ENCODE_("dot"),xml_Nodebyindex(j,node)));
			array_Remove(&tmp,&tmp[0]);
			array_Remove(&tmp,&tmp[STRLEN_(tmp)-1]);
			array_Append(&types,&tmp);
			
		};
		node=xml_Nodebyname(ENCODE_("dir"),xml_Nodebyindex(i,xml_Nodebyname(ENCODE_("mods"),config_root)));
		for(j=0;j<xml_Nodechilds(node);++j)
		{
			for(k=0;k<array_Length(types);++k)
			{
				xml_Parmbyname(&cache,ENCODE_("path"),xml_Nodebyindex(j,node));
				array_Remove(&cache,&cache[0]);
				array_Remove(&cache,&cache[STRLEN_(cache)-1]);
				mod.key=string_Create();
				array_Resize(&mod.key,1+STRLEN_(cache)+STRLEN_(types[k]));
				SNPRINTF_(mod.key,array_Head(mod.key)->array_space,HASH_KEY_,cache,types[k]);
				hash_Append(&(config->mod_table),&mod);
				PRINTF_(ENCODE_("ADDING MOD,IT'S NAME:%S\n"),mod.key);
			};
		};
		array_Drop(&types);
	};
	xml_Close(&doc);
};

void config_Drop(RESPONDER_CONFIG *config)
{
	hash_Drop(&config->mod_table);
};

BOOL_ mod_Exec(RESPONDER_CONFIG *config,HTTP_REQUEST *request,int fd)
{
	C_STRING cache;
	CHAR_ *op;
	MOD_T search;
	MOD_T *mod;

	cache=string_Create_Ex(request->path);
	for(op=&cache[STRLEN_(cache)-1];*op!=ENCODE_('.')&&(op!=cache);--op);
	for(--op;*op!=ENCODE_('/');--op)
	{
		array_Remove(&cache,op);
	};
	search.key=string_Create_Ex(cache);
	mod=hash_Get(&config->mod_table,&search);
	if(mod!=NULL)
	{
		PRINTF_(ENCODE_("NOW I WILL CALL THE MOD\n"));
		PRINTF_(ENCODE_("CALLING!\n"));
		mod->mod_Main(request,fd);
	};
	return TRUE_;

	fail_return:
	PRINTF_(ENCODE_("FAILED\n"));
	return FALSE_;
};

/*
int main(int argc,char *argv[])
{
	FILE *fp;
	RESPONDER_CONFIG config;

	if(argc>1&&argc<3)
	{
	fp=fopen(argv[1],"r");
	responder_Config(&config,fp);
	fclose(fp);
	}
	else
	{
		PRINTF_(HOWTO_);
		return 0;
	};
};
*/