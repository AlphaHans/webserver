#ifndef _WRAPPER_H__
#define _WRAPPER_H__

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/epoll.h>

int ws_socket(int family, int type, int protocol);

int ws_bind(int fd, struct sockaddr *addr, socklen_t len);

int ws_listen(int fd, int backblog);

int ws_epoll_create(int size);

int ws_set_nonblock(int fd);

int ws_set_reuseaddr(int fd);

int ws_get_port(const char *service, const char *protocol);

int ws_epoll_add_event(int epfd, int fd, int state);

int ws_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);

int ws_accept(int listenfd, struct sockaddr *addr, socklen_t *len);

int ws_epoll_mod_event(int epfd, int fd, int state);

#endif