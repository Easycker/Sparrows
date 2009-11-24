#ifndef IO_SELECT_H
#define IO_SELECT_H

#include "shttpd_type.h"
#include <stdint.h>
#include <sys/select.h>

#ifdef USE_SELECT

#define EVENT_IN 1
#define EVENT_OUT 2

/**
 * type of an event pool
 */
struct event_pool
{
	int max_fd;
	fd_set rfds;
	fd_set wfds;
};

struct event_type
{
	int fd;
	uint32_t event;
};

int eventinit(struct event_pool *pool);

int eventadd(struct event_pool *pool,int fd,uint32_t event);

int eventmod(struct event_pool *pool,int fd,uint32_t event);

int eventdel(struct event_pool *pool,int fd);

int eventwait(struct event_pool *pool,struct event_type list[],int maxfd,int timeout);

int eventclose(struct event_pool *pool);

#endif

#endif
