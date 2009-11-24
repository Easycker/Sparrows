#include "cgi.h"

UINT_ cgi_Tinyhash(CGI_ID *id)
{
	return id->fd&HASH_SPACE_;
};

BOOL_ cgi_Ensure(CGI_ID *lhs,CGI_ID *rhs)
{
	return lhs->fd==rhs->fd;
};

CGI_CONFIG* cgi_Init(IO_CONFIG *io_config,CHAR_ const *arg)
{
	XML_DOC doc;
	XML_NODE *config_root;
	XML_NODE *node;
	C_STRING cache;
	CGI_CONFIG *config;
	CGI_REGEX s_regex;
	int i;
	FILE *fp;

	fp=fopen(io_config->config_path,"r");
	if((cache=string_Create())==NULL)goto fail_return;
	if(cache==NULL)goto fail_return;
	config=(CGI_CONFIG*)malloc(sizeof(CGI_CONFIG));
	if((config->regex_list=array_Create(sizeof(CGI_REGEX)))==NULL)goto fail_return;
	xml_Open(&doc,fp,STORE_ALL);
	
	config_root=xml_Nodebyname(arg,doc.root);
	
	xml_Storedata(&cache,xml_Nodebyname(ENCODE_("buf_size"),config_root),&doc);
	config->buf_size=STRTOUL_(cache,NULL,0);

	xml_Storedata(&cache,xml_Nodebyname(ENCODE_("index_page"),config_root),&doc);
	array_Remove(&cache,&cache[0]);
	array_Remove(&cache,&cache[array_Length(cache)-2]);
	config->index_page=string_Create_Ex(cache);

	node=xml_Nodebyname(ENCODE_("dirs"),config_root);
	for(i=0;i<xml_Nodechilds(node);++i)
	{
		xml_Parmbyname(&cache,ENCODE_("regex"),xml_Nodebyindex(i,node));
		array_Remove(&cache,&cache[0]);
		array_Remove(&cache,&cache[array_Length(cache)-2]);
		if(regcomp(&s_regex.preg,cache,0)<0)goto fail_return;

		xml_Parmbyname(&cache,ENCODE_("root"),xml_Nodebyindex(i,node));
		array_Remove(&cache,&cache[0]);
		array_Remove(&cache,&cache[array_Length(cache)-2]);
		s_regex.exec_path=string_Create_Ex(cache);

		xml_Parmbyname(&cache,"parse",xml_Nodebyindex(i,node));
		if(array_Length(cache)>3)
		{
			array_Remove(&cache,&cache[0]);
			array_Remove(&cache,&cache[array_Length(cache)-2]);
			s_regex.parse=string_Create_Ex(cache);
			ERROR_OUT_(stderr,"add a parse:%s\n",cache);
		}
		else
		{
			s_regex.parse=NULL;
		}

		xml_Parmbyname(&cache,"type",xml_Nodebyindex(i,node));
		if(array_Length(cache)>3)
		{
			array_Remove(&cache,&cache[0]);
			array_Remove(&cache,&cache[array_Length(cache)-2]);
			s_regex.type=string_Create_Ex(cache);
			ERROR_OUT_(stderr,"add a type:%s\n",cache);
		}
		else
		{
			s_regex.type=NULL;
		}

		array_Append(&config->regex_list,&s_regex);
	};

	/*Init the hash table*/
	hash_Create(&config->id_list,&cgi_Tinyhash,&cgi_Ensure,sizeof(CGI_ID),(UINT_)HASH_SPACE_+1);
	dlist_Create(&config->owner_list,sizeof(CGI_OWNER));
	
	string_Drop(&cache);
	xml_Close(&doc);

	fclose(fp);

	return config;
fail_return:
	return NULL;
};

int cgi_Select(CGI_CONFIG *config,HTTP_REQUEST *request,int fd)
{
	C_STRING cache;
	C_STRING parse;
	C_STRING type;
	int i;
	pid_t pid;
	regmatch_t preg_match;
	CGI_ID connect_id;
	CGI_OWNER connect_owner;
	CGI_OWNER *id_owner;
	struct stat file_stat;
	int f_in[2];
	int f_out[2];
	char *ptr;
	char header_cache[256];

	cache=string_Create();
	for(i=0;i<array_Length(config->regex_list);++i)
	{
		if(!regexec(&config->regex_list[i].preg,request->path,1,&preg_match,0))
		{
			ERROR_OUT_(stderr,ENCODE_("THE PATH IS %s\n"),request->path);
			if(array_Resize(&cache,array_Length(request->path)+array_Length(config->regex_list[i].exec_path))==NULL)goto fail_return;
			snprintf(cache,array_Head(cache)->array_space,ENCODE_("%s%s"),config->regex_list[i].exec_path,request->path);
			array_Length(cache)=STRLEN_(cache)+1;
			parse=config->regex_list[i].parse;
			type=config->regex_list[i].type;

			if(cache[array_Length(cache)-2]==ENCODE_('/'))
			{
				ERROR_OUT_(stderr,ENCODE_("USE DEFAULT INDEX CGI\n"));
				array_Remove(&cache,&cache[array_Length(cache)-1]);
				for(i=0;i<array_Length(config->index_page);++i)if(string_Append(&cache,&config->index_page[i])==NULL)goto fail_return;
			};
			ERROR_OUT_(stderr,ENCODE_("STATING %s\n"),cache);

			if(pipe(f_in)<0||pipe(f_out)<0)
			{
				ERROR_OUT_(stderr,ENCODE_("FAIL WHILE CREATE PIPE\n"));
				goto fail_return;
			};
			fd_Setnonblocking(f_in[0]);
			fd_Setnonblocking(f_in[1]);
			fd_Setnonblocking(f_out[0]);
			fd_Setnonblocking(f_out[1]);

			pid=fork();
			if(pid)
			{
				/*fathcer process*/

				connect_owner.pid=pid;
				close(f_out[1]);
				close(f_in[0]);
				connect_owner.cgi_in=f_in[1];
				connect_owner.cgi_out=f_out[0];

				connect_owner.in_fd=fd;
				connect_owner.in_read=FALSE_;
				connect_owner.in_write=FALSE_;
				connect_owner.remote_read=FALSE_;
				connect_owner.remote_write=FALSE_;
				connect_owner.last_op=NOTHING_;
				connect_owner.len=0;

				id_owner=dlist_Append(&connect_owner,&config->owner_list);
				connect_id.owner=id_owner;
				connect_id.fd=fd;
				if(hash_Append(&config->id_list,&connect_id)==NULL)
				{
					ERROR_OUT_(stderr,ENCODE_("ERROR WHILE APPEND ID\n"));
					goto fail_return;
				};
				connect_id.fd=connect_owner.cgi_in;
				if(hash_Append(&config->id_list,&connect_id)==NULL)
				{
					ERROR_OUT_(stderr,ENCODE_("ERROR WHILE APPEND ID\n"));
					goto fail_return;
				};
				connect_id.fd=connect_owner.cgi_out;
				if(hash_Append(&config->id_list,&connect_id)==NULL)
				{
					ERROR_OUT_(stderr,ENCODE_("ERROR WHILE APPEND ID\n"));
					goto fail_return;
				};
				config->new_fd1=connect_owner.cgi_in;
				config->new_fd2=connect_owner.cgi_out;

				/*checking whether there are post datas*/
				if(request->type==POST)
				{
					ptr=strstr(request->recv_data,"\r\n\r\n");
					if(ptr!=NULL)ptr+=4;
					write(connect_owner.cgi_in,ptr,request->recv_data+request->recv_len-ptr);
				}
				
				string_Drop(&cache);
				return SELECT_OUTPUT_|SELECT_INPUT_|SELECT_NEWPORT_;
			}
			else
			{
				if(request->query_string!=NULL)setenv("QUERY_STRING",request->query_string,0);
				if(request->cookies!=NULL)setenv("HTTP_COOKIES",request->cookies,0);
				if(request->accept!=NULL)setenv("ACCEPT",request->accept,0);
				if(request->accept_encoding!=NULL)setenv("ACCEPT_ENCODING",request->accept_encoding,0);
				if(request->accept_language!=NULL)setenv("ACCEPT_LANGUAGE",request->accept_language,0);
				if(request->referer!=NULL)setenv("HTTP_REFERER",request->referer,0);
				if(request->content_length!=NULL)setenv("CONTENT_LENGTH",request->content_length,0);
				if(request->content_type!=NULL)setenv("CONTENT_TYPE",request->content_type,0);

				/*child process*/
				dup2(f_out[1],fileno(stdout));
				dup2(f_in[0],fileno(stdin));
				close(f_out[1]);
				close(f_out[0]);
				close(f_in[1]);
				close(f_in[0]);
				ERROR_OUT_(stderr,"THE EXEC CGI IS %s\n",cache);
				if(stat(cache,&file_stat)<0)
				{
					ERROR_OUT_(stderr,"write 404 fail\n");
					/*can here call the static?*/
					write(fileno(stdout),HTTP_NOT_FOUND_MSG_,strlen(HTTP_NOT_FOUND_MSG_));
				}
				else
				{
					ERROR_OUT_(stderr,"write 200 ok\n");
					if(type!=NULL)
					{
						ERROR_OUT_(stderr,"with type\n");
						ERROR_OUT_(stderr,HTTP_OK_TYPE_,type);
						sprintf(header_cache,HTTP_OK_TYPE_,type);
						write(fileno(stdout),header_cache,strlen(header_cache));
					}
					else
					{
						ERROR_OUT_(stderr,"no type\n");
						write(fileno(stdout),HTTP_OK_,strlen(HTTP_OK_));
					}
				}
				ptr=strrchr(cache,'/');
				if(ptr!=NULL)
				{
					*ptr='\0';
					chdir(cache);
					*ptr='/';
					++ptr;
				}
				else
				{
					ptr=cache;
				}
				if(parse!=NULL)
				{
					ERROR_OUT_(stderr,"use parse:%s,to exec:%s\n",parse,ptr);
					execlp(parse,parse,ptr,(char*)0);
				}
				else execl(ptr,ptr,(char*)0);
				ERROR_OUT2_(stderr,ENCODE_("FAIL WHILE FORK\n"));
				ERROR_EXIT_;
				exit(1);
			};
		};
	};
	string_Drop(&cache);
	return SELECT_GOON_;
fail_return:
	
	ERROR_OUT2_(stderr,ENCODE_("FAIL WHILE SELECT CGI\n"));
	ERROR_PRINT_;
	exit(1);
	
	return SELECT_BREAK_;
};

int cgi_Work(CGI_CONFIG *config,HTTP_CONNECT *connect)
{
	CGI_ID fake_id;
	CGI_ID *id;
	int len;

	ERROR_OUT_(stderr,ENCODE_("MOD_CGI STARTED\n"));
	fake_id.fd=connect->fd;
	id=hash_Get(&config->id_list,&fake_id);
	if(id!=NULL)
	{
		if(id->fd==id->owner->in_fd)
		{
			if(connect->event&WORK_INPUT_)
			{
				ERROR_OUT_(stderr,ENCODE_("SET IN_READ TO TRUE\n"));
				id->owner->in_read=TRUE_;
			};
			if(connect->event&WORK_OUTPUT_)
			{
				ERROR_OUT_(stderr,ENCODE_("SET IN_WRITE TO TRUE\n"));
				id->owner->in_write=TRUE_;
			};
		}
		else if(id->fd==id->owner->cgi_in)
		{
			ERROR_OUT_(stderr,ENCODE_("SET CGI_WRITE TO TRUE\n"));
			id->owner->remote_write=TRUE_;
		}
		else if(id->fd==id->owner->cgi_out)
		{
			ERROR_OUT_(stderr,ENCODE_("SET CGI_READ TO TRUE\n"));
			id->owner->remote_read=TRUE_;
		}
		else goto fail_return;

		if(id->owner->in_read==TRUE_&&id->owner->remote_write==TRUE_)
		{
#ifdef SPLICE_ENABLE
			ERROR_OUT_(stderr,ENCODE_("START SEND DATA TO REMOTE\n"));

			/*local machine send message to remote machine*/
			if(id->owner->last_op==NOTHING_||id->owner->last_op==LOCAL_SEND_REMOTE_)
			{
				/*ok start work!*/
				do
				{
					if((len=splice(id->owner->in_fd,NULL,id->owner->cgi_in,NULL,config->buf_size,SPLICE_F_NONBLOCK))<0)
					{
						if(errno==EAGAIN)
						{
							id->owner->in_read=FALSE_;
							id->owner->last_op=NOTHING_;
							break;
						}
						else
						{
							ERROR_PRINT_;
							goto fail_return;
						};
					};
					id->owner->len+=len;
					if(len==0)
					{
						ERROR_OUT_(stderr,ENCODE_("SPLICE RETURN 0,CONNECT HAD BEEN CLOSE\n"));
						ERROR_PRINT_;
						return WORK_CLOSE_;
					};
				}while(len>0);
			};
#else
#endif
		};
		if(id->owner->in_write==TRUE_&&id->owner->remote_read==TRUE_)
		{
#ifdef SPLICE_ENABLE
			ERROR_OUT_(stderr,ENCODE_("START SEND DATA FROM REMOTE TO LOCAL\n"));
			if(id->owner->last_op==NOTHING_||id->owner->last_op==REMOTE_SEND_LOCAL_)
			{
				/*ok start work!*/
				ERROR_OUT_(stderr,ENCODE_("REMOTE LOOP START\n"));
				do
				{
					if((len=splice(id->owner->cgi_out,NULL,id->owner->in_fd,NULL,config->buf_size,SPLICE_F_NONBLOCK))<0)
					{
						if(errno==EAGAIN)
						{
							if(id->owner->len>0)
							{
								id->owner->in_write=FALSE_;
								id->owner->last_op=REMOTE_SEND_LOCAL_;
							}
							else
							{
								id->owner->last_op=NOTHING_;
							};
							break;
						}
						else
						{
							ERROR_PRINT_;
							goto fail_return;
						};
					};
					ERROR_OUT_(stderr,ENCODE_("SEND %d data\n"),len);
					if(len==0)
					{
						ERROR_OUT_(stderr,ENCODE_("SPLICE RETURN 0,CONNECT HAD BEEN CLOSE\n"));
						ERROR_PRINT_;
						return WORK_CLOSE_;
					};
					id->owner->len-=len;
				}while(len>0);
			};
#else
#endif
		};
	};
	return WORK_GOON_;

fail_return:
	/*ERROR_PRINT_;*/
	return WORK_CLOSE_;
};

int cgi_Addport(CGI_CONFIG *config,HTTP_CONNECT *connect,PORT_APPLY *apply)
{
	if(config->new_fd1>0)
	{
		apply->fd=config->new_fd1;
		config->new_fd1=-1;
		return WORK_OUTPUT_|WORK_NEWPORT_;
	}
	else
	{
		apply->fd=config->new_fd2;
		config->new_fd2=-1;
		return WORK_INPUT_;
	};
};

int cgi_Closeport(CGI_CONFIG *config,HTTP_CONNECT *connect,PORT_APPLY *apply)
{
	if(config->close_fd1>0)
	{
		ERROR_OUT_(stderr,ENCODE_("CLOSEPORT RECV THE FD %d\n"),config->close_fd1);
		apply->fd=config->close_fd1;
		config->close_fd1=-1;
		return 1;
	}
	else
	{
		ERROR_OUT_(stderr,ENCODE_("CLOSEPORT RECV THE FD %d\n"),config->close_fd2);
		apply->fd=config->close_fd2;
		config->close_fd2=-1;
		return -1;
	};
};

int cgi_Close(CGI_CONFIG *config,HTTP_CONNECT *connect)
{
	CGI_ID fake_id;
	CGI_ID *id;
	CGI_ID *cgiout_id;
	CGI_ID *cgiin_id;
	CGI_ID *in_id;

	ERROR_OUT_(stderr,ENCODE_("CLOSEING CGI,connect_fd is %d\n"),connect->fd);
	fake_id.fd=connect->fd;
	id=hash_Get(&config->id_list,&fake_id);
	if(id!=NULL)
	{
		if(id->fd==id->owner->cgi_in)
		{
			config->close_fd1=id->owner->cgi_out;
			config->close_fd2=id->owner->in_fd;
		}
		else if(id->fd==id->owner->in_fd)
		{
			config->close_fd1=id->owner->cgi_out;
			config->close_fd2=id->owner->cgi_in;
		}
		else if(id->fd==id->owner->cgi_out)
		{
			config->close_fd1=id->owner->cgi_in;
			config->close_fd2=id->owner->in_fd;
		}
		else goto fail_return;

		ERROR_OUT_(stderr,ENCODE_("CLOSEING PIPE\n"));
		fake_id.fd=id->owner->cgi_in;
		if((cgiin_id=hash_Get(&config->id_list,&fake_id))==NULL)
		{
			ERROR_OUT_(stderr,ENCODE_("CGI_IN FD RETURNS NULL\n"));
		};
		fake_id.fd=id->owner->in_fd;
		if((in_id=hash_Get(&config->id_list,&fake_id))==NULL)
		{
			ERROR_OUT_(stderr,ENCODE_("IN FD RETURNS NULL\n"));
		};
		fake_id.fd=id->owner->cgi_out;
		if((cgiout_id=hash_Get(&config->id_list,&fake_id))==NULL)
		{
			ERROR_OUT_(stderr,ENCODE_("CGI_OUT FD RETURNS NULL\n"));
		};
		dlist_Remove(id->owner,&config->owner_list);
		hash_Remove(&config->id_list,in_id);
		hash_Remove(&config->id_list,cgiin_id);
		hash_Remove(&config->id_list,cgiout_id);
		kill(id->owner->pid,SIGKILL);
		return WORK_CLOSEPORT_;
	};
	return 0;
fail_return:
	ERROR_OUT_(stderr,ENCODE_("CONNECT ID ISN'T IN OWNER\n"));
	exit(1);
	ERROR_PRINT_;
	return 0;
};

void cgi_Unload(CGI_CONFIG *config)
{
	ERROR_OUT_(stderr,ENCODE_("I KILL MYSELF!\n"));
	return;
};
