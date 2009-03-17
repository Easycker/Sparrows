#include "responder.h"

RESPONDER_CONFIG* responder_Config(RESPONDER_CONFIG *config,FILE *fp)
{
	XML_DOC doc;
	XML_NODE *config_root;
	XML_NODE *node;
	C_STRING cache;
	C_ARRAY C_STRING *types;
	UINT_ i;
	UINT_ j;
	UINT_ k;
	MOD_T mod;
	HASH_MOD mod_hash;

	cache=string_Create();
	xml_Open(&doc,fp,STORE_ALL);
	config_root=xml_Nodebyname(ENCODE_("responder"),doc.root);
	xml_Storedata(&cache,xml_Nodebyname(ENCODE_("port"),config_root),&doc);
	config->port=STRTOUL_(cache,NULL,0);
	for(i=0;i<xml_Nodechilds(xml_Nodebyname(ENCODE_("mods"),config_root);++i)
	{
		types=array_Create_Ex(sizeof(C_STRING),&string_Drop);
		node=xml_Nodebyname(ENCODE_("type"),xml_Nodebyindex(i,xml_Nodebyname(ENCODE_("mods"),config_root)));
		for(j=0;j<xml_Nodechilds(node);++j)types=array_Append(types,&string_Create_Ex(xml_Parmbyname(&cache,ENCODE_("dot"),xml_Nodebyindex(j,node))));
		node=xml_Nodebyname(ENCODE_("dir"),xml_Nodebyindex(i,xml_Nodebyname(ENCODE_("mods"),config_root)));
		for(j=0;j<xml_Nodechilds(node);++j)
		{
			for(k=0;k<array_Length(types);++k)
			{
				cache=array_Resize(cache,1+STRLEN_(xml_Parmbyname(&cache,ENCODE_("path"),xml_Nodebyindex(j,node)))+STRLEN_(types[k]));
				SPRINTF_(cache,HASH_KEY_,xml_Parmbyname(&cache,ENCODE_("path"),xml_Nodebyindex(j,node)),types[k]);
				
			};
		};
	};

	types=array_Create_Ex(sizeof(C_STRING),&string_Drop);
	for(i=0;i<xml_Nodechilds(node);++i)
	{
		
	};
};

int main(int argc,char *argv[])
{
	
};