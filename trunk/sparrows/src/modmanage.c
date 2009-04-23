#include "modmanage.h"

MODMANAGE_CONFIG* modmanage_Init(MODMANAGE_CONFIG *config,FILE *fp)
{
	XML_DOC doc;
	XML_NODE *config_root;
	XML_NODE *node;
	C_STRING cache;
	C_ARRAY char *cache_ansi;
	UINT_ i;
	UINT_ j;
	UINT_ k;
	MOD_T mod;
	void *share_mod;
	FILE *fp_;

	cache=string_Create();
	cache_ansi=array_Create(sizeof(char));
	xml_Open(&doc,fp,STORE_ALL);
	config_root=xml_Nodebyname(ENCODE_("modmanage"),doc.root);
	config->mod_table=array_Create(sizeof(MOD_T));
	/*read the mods*/
	for(i=0;i<xml_Nodechilds(xml_Nodebyname(ENCODE_("mods"),config_root));++i)
	{
		/*clean up the mod_t*/
		xml_Parmbyname(&cache,ENCODE_("path"),xml_Nodebyindex(i,xml_Nodebyname(ENCODE_("mods"),config_root)));
		array_Remove(&cache,&cache[0]);
		array_Remove(&cache,&cache[STRLEN_(cache)-1]);

		string_Widetoansi(&cache_ansi,cache);

		share_mod=dlopen(cache_ansi,RTLD_LAZY);
		mod.mod_lib=share_mod;
		
		mod.mod_Init=dlsym(share_mod,"mod_Init");
		mod.mod_Select=dlsym(share_mod,"mod_Select");
		mod.mod_Work=dlsym(share_mod,"mod_Work");
		mod.mod_Addport=dlsym(share_mod,"mod_Addport");
		mod.mod_Unload=dlsym(share_mod,"mod_Unload");

		PRINTF_(ENCODE_("DLERROR:%s\n"),dlerror());

		xml_Parmbyname(&cache,ENCODE_("arg"),xml_Nodebyindex(i,xml_Nodebyname(ENCODE_("mods"),config_root)));
		array_Remove(&cache,&cache[0]);
		array_Remove(&cache,&cache[STRLEN_(cache)-1]);

		mod.share=mod.mod_Init(cache);

		array_Append(&config->mod_table,&mod);
	};
	xml_Close(&doc);
	string_Drop(&cache);
	array_Drop(&cache_ansi);
};

void modmanage_Drop(MODMANAGE_CONFIG *config)
{
	array_Drop(&config->mod_table);
};
