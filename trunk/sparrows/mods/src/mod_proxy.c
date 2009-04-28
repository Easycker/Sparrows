#include "mod_proxy.h"

UINT_ porxy_Tinyhash(PROXY_ID *id)
{
	uint32_t hash=0;
	uint32_t x=0;
	int i;
	char *op;
	size_t len;

	len=sizeof(id->connect_fd);
	op=(char*)&(id->connect_fd);
	for(i=0;i<len;++i)
	{
		hash=(hash<<4)+(*op++);
		if((x=hash&0xF0000000L)!=0)
		{
			hash^= (x>>24);
			hash&=~x;
		};
	};
	return (hash&0x0000ffff);
};

BOOL_ proxy_Ensure(PROXY_ID *lhs,PROXY_ID *rhs)
{
	return lhs->connect_fd==rhs->connect_fd;
};

PROXY_CONFIG* mod_Init(CHAR_ const *arg)
{
};

int mod_Select(PROXY_CONFIG *config,HTTP_REQUEST *request,int fd)
{
};

int mod_Work(PROXY_CONFIG *config,HTTP_CONNECT *connect)
{
};

int mod_Addport(PROXY_CONFIG *config,HTTP_CONNECT *connect,PORT_APPLY *apply)
{
};

void mod_Unload(PROXY_CONFIG *config)
{
};
