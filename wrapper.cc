#include "wrapper.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

void err_quit(const char *why)
{
	perror(why);
	exit(-1);
}

int ws_socket(int family, int type, int protocol)
{
	int ret;
	ret = socket(family, type, protocol);
	if (ret == -1) err_quit("socket error");
	else return ret;
}

int ws_set_nonblock(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		err_quit("fcntl fd_getfl error");

	flags |= O_NONBLOCK;
	int ret = fcntl(fd, F_SETFL, flags);
	if (ret < 0)
		err_quit("fcntl fd_setfl error");
	return ret;
}

int ws_set_reuseaddr(int fd)
{
	int on = 1, ret;
	ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	if (ret < 0)
		err_quit("setsockopt SO_REUSEADDR error");
	return ret;
}

int ws_get_port(const char *service, const char *protocol)
{
	struct servent *pservent;
	if ( (pservent = getservbyname(service, protocol)) == NULL )
		err_quit("getservbyname error");
	return pservent->s_port;
}

int ws_bind(int fd, struct sockaddr *addr, socklen_t len)
{
	int ret;
	if ((ret = bind(fd, addr, len)) == -1)
		err_quit("bind error");
	return ret;
}

int ws_listen(int fd, int backblog)
{
	int ret = 0;
	if ((ret = listen(fd, backblog)) < 0)
		err_quit("listen error");
	return ret;
}

int ws_epoll_create(int size)
{
	int ret;
	if ((ret = epoll_create(size)) < 0)
		err_quit("epoll_create error");
	return ret;
}

int ws_epoll_add_event(int epfd, int fd, int state)
{
	int ret;
	struct epoll_event ev;
	ev.events = state;
	ev.data.fd = fd;

	if ((ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev)) < 0)
		err_quit("epoll_ctl add error");

	return ret;
}

int ws_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
again:
	int ret;
	if ((ret = epoll_wait(epfd, events, maxevents, timeout)) < 0)
	{
		if (errno == EINTR) goto again;
		else perror("epoll_wait error");
	}
	return ret;
}

int ws_accept(int listenfd, struct sockaddr *addr, socklen_t *len)
{	
again:
	int ret;
	if ((ret = accept(listenfd, addr, len)) < 0)
	{
		if (errno == EINTR)
			goto again;
		else
			err_quit("accept error");
	}

	return ret;
}

int ws_epoll_mod_event(int epfd, int fd, int state)
{
	struct epoll_event ev;
	ev.events = state;
	ev.data.fd = fd;
	int ret;
	if ((ret = epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev)) < 0)
		err_quit("epoll_ctl error");
	else
		return ret;
}