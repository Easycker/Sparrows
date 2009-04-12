#include <stdio.h>
#include <cda/c_string.h>
#include <xmlo/xmlo.h>

int main()
{
	XML_DOC my_xml;
	XML_NODE *my_node;
	size_t n;
	size_t m;
	FILE *file;
	C_STRING parm;

	parm=string_Create();
	file=fopen("testxml","r");
	SET_LOCALE_("");
	PRINTF_(ENCODE_("NOW START\n"));
	if(xml_Open(&my_xml,file,STORE_ALL)==NULL)PRINTF_(ENCODE_("Open fail\n"));

	my_node=xml_Nodebyname(ENCODE_("EZLearn_MultiFile_List"),my_xml.root);
	/*
	n=xml_Nodechilds(my_xml.root);
	PRINTF_(ENCODE_("IT HAVE %lu CHILDS\n"),n);
	*/
	if(my_node==NULL)PRINTF_(ENCODE_("FAILED\n"));
	PRINTF_(ENCODE_("NOW I GET IT IT\'S NAME IS %S\n"),xml_Nodename(&parm,my_node));

	file=fopen("xmloutput","w");
	xml_Save(file,&my_xml);
	fclose(file);

	xml_Close(&my_xml);
	string_Drop(parm);

	return 0;
};