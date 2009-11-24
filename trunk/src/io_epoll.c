#include "io_epoll.h"

#ifdef USE_EPOLL

int eventinit(struct event_pool *pool)
{
	assert(pool!=NULL);
	pool->epoll_fd=epoll_create(POOL_LENGTH);
	return 0;
}

int eventadd(struct event_pool *pool,int fd,uint32_t event)
{
	struct epoll_event io_event;

	assert(pool!=NULL);
	memset(&io_event,0,sizeof(struct epoll_event));
	if(event&EVENT_IN)io_event.events|=EPOLLIN;
	if(event&EVENT_OUT)io_event.events|=EPOLLOUT;
	io_event.events|=EPOLLET;
	io_event.data.fd=fd;

	return epoll_ctl(pool->epoll_fd,EPOLL_CTL_ADD,fd,&io_event);
}

int eventmod(struct event_pool *pool,int fd,uint32_t event)
{
	struct epoll_event io_event;

	memset(&io_event,0,sizeof(io_event));
	io_event.data.fd=fd;
	if(event&EVENT_IN)io_event.events|=EPOLLIN;
	if(event&EVENT_OUT)io_event.events|=EPOLLOUT;
	io_event.events|=EPOLLET;
	return epoll_ctl(pool->epoll_fd,EPOLL_CTL_MOD,fd,&io_event);
}

int eventdel(struct event_pool *pool,int fd)
{
	struct epoll_event io_event;

	assert(pool!=NULL);
	memset(&io_event,0,sizeof(struct epoll_event));
	io_event.data.fd=fd;
	return epoll_ctl(pool->epoll_fd,EPOLL_CTL_DEL,fd,&io_event);
}

int eventwait(struct event_pool *pool,struct event_type list[],int maxfd,int timeout)
{
	struct epoll_event events[POOL_LENGTH];
	int count;
	int i;

	assert(pool!=NULL&&list!=NULL);
	if(maxfd>POOL_LENGTH)return -1;

	count=epoll_wait(pool->epoll_fd,events,maxfd,timeout);
	if(count>0)
	{
		for(i=0;i<count;++i)
		{
			list[i].fd=events[i].data.fd;
			list[i].event=0;
			if(events[i].events&EPOLLIN)list[i].event|=EVENT_IN;
			if(events[i].events&EPOLLOUT)list[i].event|=EVENT_OUT;
		}
		return count;
	}
	else
	{
		return count;
	}
}

int eventclose(struct event_pool *pool)
{
	assert(pool!=NULL);
	return close(pool->epoll_fd);
}

#endif
