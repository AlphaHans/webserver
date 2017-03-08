#include "jhhttp.h"
#include "wrapper.h"
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <string>
#include "httpapi.h"

using namespace std;

#define MAXSIZE 2014*1024

void handle_request(int epfd, int fd);

int main(int argc, char const *argv[])
{
	int listenfd, connfd, epfd;
	int listenport;
	struct sockaddr_in servaddr, cliaddr;
	socklen_t clilen;
	struct epoll_event events[MAX_EVENTS];
	//config

	listenfd = ws_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
	printf("listen fd = %d\n", listenfd);
	//ws_set_nonblock(listenfd);	//for epoll et
	ws_set_reuseaddr(listenfd);	// Socket operation on non-socket
	//listenport = ws_get_port("http", "tcp");
	listenport = 9877;
	printf("PORT = %d\n", listenport);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(listenport);

	ws_bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	ws_listen(listenfd, BACKBLOG);

	epfd = ws_epoll_create(MAX_EVENTS);

	ws_epoll_add_event(epfd, listenfd, EPOLLIN);

	//loop
	for ( ; ; )
	{
		int nready = ws_epoll_wait(epfd, events, MAX_EVENTS, -1);
		//epill_wait被中断则重新调用该函数
		if (nready == -1 && errno == EINTR)
			continue;

		for (int i = 0;i < nready; ++i)
		{
			int fd = events[i].data.fd;
			if (fd == listenfd && (events[i].events & EPOLLIN))
			{
				//new client
				connfd = ws_accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);
				//ws_set_nonblock(connfd);
				ws_epoll_add_event(epfd, connfd, EPOLLIN);
			}
			else if (events[i].events & EPOLLIN)
			{
				//data arrive
				handle_request(epfd, fd);
			}
		}
	}

	close(epfd);
	exit(0);
}

void handle_request(int epfd, int sockfd)
{
	char buf[MAXSIZE + 1];
	int nrecv;

	nrecv = recv(sockfd, buf, sizeof(buf), 0);	//request package
	buf[nrecv] = '\0';

	if (nrecv > 0)
	{
		ws_epoll_mod_event(epfd, sockfd, EPOLLOUT);
		ws_http_request_t request;
		parse_http_header(&request, string(buf));
		db_print_header(&request);
		http_response(sockfd, &request);
	}
}