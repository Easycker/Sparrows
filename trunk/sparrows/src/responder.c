#include "responder.h"

unsigned char elf_Tinyhash(MOD_T *mod)
{
	C_STRING string;
	uint32_t hash=0;
	uint32_t x=0 ;
	char *op;
	size_t len;

	string=mod->key;
	len=(array_Length(string)+1)*sizeof(CHAR_);
	op=(char*)string;
	while(len--)
	{
		hash=(hash<<4)+(*op++);
		if((x=hash&0xF0000000L)!=0)
		{
			hash^= (x>>24);
			hash&=~x;
		};
	};
	return (unsigned char)(hash&255);
	/*255=0x000000ff*/
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
	hash_Create(&(config->mod_table),&elf_Tinyhash,&STRCMP_,sizeof(MOD_T*),255);
	for(i=0;i<xml_Nodechilds(xml_Nodebyname(ENCODE_("mods"),config_root));++i)
	{
		PRINTF_(ENCODE_("NOW START THE %d\n"),i);

		/*clean up the mod_t*/
		mod.key=string_Create();
		mod.mod_Main=NULL;
		types=array_Create_Ex(sizeof(C_STRING),&string_Drop);
		/*load the module*/

		xml_Parmbyname(&cache,ENCODE_("path"),xml_Nodebyindex(i,xml_Nodebyname(ENCODE_("mods"),config_root)));

		cache=array_Remove(cache,&cache[0]);
		cache=array_Remove(cache,&cache[array_Length(cache)-1]);
		string_Widetoansi(&cache_ansi,cache);
		
		/*
		share_mod=dlopen(cache_ansi,RTLD_LAZY);
		mod.mod_Main=dlsym(share_mod, "mod_Main");
		*/

		node=xml_Nodebyname(ENCODE_("type"),xml_Nodebyindex(i,xml_Nodebyname(ENCODE_("mods"),config_root)));
		for(j=0;j<xml_Nodechilds(node);++j)
		{
			tmp=string_Create_Ex(xml_Parmbyname(&cache,ENCODE_("dot"),xml_Nodebyindex(j,node)));
			
			types=array_Append(types,&tmp);
		};
		node=xml_Nodebyname(ENCODE_("dir"),xml_Nodebyindex(i,xml_Nodebyname(ENCODE_("mods"),config_root)));
		for(j=0;j<xml_Nodechilds(node);++j)
		{
			for(k=0;k<array_Length(types);++k)
			{
				mod.key=array_Resize(mod.key,1+STRLEN_(xml_Parmbyname(&cache,ENCODE_("path"),xml_Nodebyindex(j,node)))+STRLEN_(types[k]));
				SNPRINTF_(mod.key,array_Length(mod.key),HASH_KEY_,xml_Parmbyname(&cache,ENCODE_("path"),xml_Nodebyindex(j,node)),types[k]);
				hash_Append(&(config->mod_table),&mod);
			};
		};
		array_Drop(types);
	};

};

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