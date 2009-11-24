#include "io_select.h"

#ifdef USE_SELECT

int eventinit(struct event_pool *pool)
{
	assert(pool!=NULL);
	FD_ZERO(&pool->wfds);
	FD_ZERO(&pool->rfds);
	pool->max_fd=0;
	return 0;
}

int eventadd(struct event_pool *pool,int fd,uint32_t event)
{
	assert(pool!=NULL);
	if(event&EVENT_IN)FD_SET(fd,&pool->rfds);
	if(event&EVENT_OUT)FD_SET(fd,&pool->wfds);
	if(fd>(pool->max_fd))pool->max_fd=fd;
	return 0;
}

int eventmod(struct event_pool *pool,int fd,uint32_t event)
{
	assert(pool!=NULL);
	if((event&EVENT_IN)&&(event&EVENT_OUT))
	{
		FD_SET(fd,&pool->rfds);
		FD_SET(fd,&pool->wfds);
	}
	else if(event&EVENT_IN)
	{
		FD_SET(fd,&pool->rfds);
		FD_CLR(fd,&pool->wfds);
	}
	else if(event&EVENT_OUT)
	{
		FD_SET(fd,&pool->wfds);
		FD_CLR(fd,&pool->rfds);
	}
	else
	{
		return eventdel(pool,fd);
	}
	return 0;
}

int eventdel(struct event_pool *pool,int fd)
{
	assert(pool!=NULL);
	FD_CLR(fd,&pool->rfds);
	FD_CLR(fd,&pool->wfds);
	return 0;
}

int eventwait(struct event_pool *pool,struct event_type list[],int maxfd,int timeout)
{
	struct timeval timeout_set;
	int ret;
	int n;
	int i;
	fd_set rfds;
	fd_set wfds;

	assert(pool!=NULL&&list!=NULL);
	timeout_set.tv_sec=0;
	timeout_set.tv_usec=timeout*1000;

	memcpy(&rfds,&pool->rfds,sizeof(fd_set));
	memcpy(&wfds,&pool->wfds,sizeof(fd_set));

	ret=select(pool->max_fd+1,&rfds,&wfds,NULL,&timeout_set);
	if(ret>0)
	{
		n=0;
		list[n].fd=0;
		list[n].event=0;
		for(i=0;(i<pool->max_fd+1)&&(i<maxfd);++i)
		{
			if(FD_ISSET(i,&pool->rfds))list[n].event|=EVENT_IN;
			if(FD_ISSET(i,&pool->wfds))list[n].event|=EVENT_OUT;
			if(list[n].event!=0)
			{
				list[n].fd=i;
				++n;
				list[n].event=0;
				list[n].fd=0;
			}
		}
		return n+1;
	}
	return 0;
}

int eventclose(struct event_pool *pool)
{
	return 0;
}

#endif
