#ifndef HTTP_API_H__
#define HTTP_API_H__ 

#include <string>
#include <map>
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/stat.h>

using namespace std;

//http status code
#define WS_CONTINUE 	100	//收到了请求的起始部分，客户端应该继续请求
#define WS_OK			200	//服务器已经成功处理请求
#define WS_ACCEPTED		202	//请求已接受，服务器尚未处理
#define WS_MOVED		301	//请求的URL已移走，响应应该包含Location URL
#define	 WS_FOUND		302	//请求的URL临时移走，响应应该包含Location URL
#define WS_SEEOTHER		303	//告诉客户端应该用另一个URL获取资源，响应应该包含Location URL
#define WS_NOTMODIFIED	304	//资源未发生变化
#define WS_BADREQUEST	400	//客户端发送了一条异常请求
#define WS_FORBIDDEN	403	//服务器拒绝请求
#define WS_NOTFOUND	404	//URL未找到
#define WS_ERROR		500	//服务器出错
#define WS_NOIMPLEMENTED	501 //服务器不支持当前请求所需要的某个功能
#define WS_BADGATEWAY	502	//作为代理或网关使用的服务器遇到了来自响应链中上游的无效响应
#define WS_SRVUNAVAILABLE	503 //服务器目前无法提供请求服务，过一段时间后可以恢复

//http reason phrase
const char ws_ok[] = "OK";
const char ws_badrequest[] =  "Bad Request";
const char ws_forbidden[] = "Forbidden";
const char ws_notfound[] =  "Not Found";
const char ws_noimplemented[] = "No Implemented";

//http header key
#define WS_ACCEPTRANGE_HEAD		"Accpet-Range:"
#define WS_AGE_HEAD 			"Age:"
#define	 WS_ALLOW_HEAD			"Allow:"
#define	 WS_CONTENTBASE_HEAD		"Content-Base:"
#define WS_CONTENTLENGTH_HEAD		"Content-Length:"
#define	 WS_CONTENTLOCATION_HEAD	"Content-Location:"
#define	 WS_CONTENTRANGE_HEAD		"Content-Range:"
#define	 WS_CONTENTTYPE_HEAD		"Content-Type:"
#define WS_DATE_HEAD			"Date:"
#define WS_EXPIRES_HEAD			"Expires:"
#define WS_LAST_MODIFIED_HEAD		"Last-Modified:"
#define WS_LOCATION_HEAD 			"Location:"
#define	 WS_PUBLIC_HEAD			"Public:"
#define WS_RANGE_HEAD 			"Range:"
#define WS_SERVER_HEAD			"Server:"
#define WS_CRLF				"\r\n"

#define LOACL_HTTP_VERSION "HTTP/1.1"

typedef std::map<string, std::string> ws_header;

typedef struct ws_http_request 
{
	//request line
	std::string method;
	std::string url;
	std::string version;
	//header line
	ws_header header;
	//body
	std::string entitybody;

} ws_http_request_t;

typedef struct ws_http_response
{
	//status line
	std::string version;
	std::string statuscode;
	std::string phrase;
	//header
	ws_header header;
	void* entitybody;	
} ws_http_response_t;

int parse_http_header(ws_http_request *preq, const std::string &data)
{
	if (data.empty())
	{
		perror("parse_http_header: data is empty");
		return -1;
	}
	if (preq == NULL) 
	{
		perror("parse_http_header: request is null");
		return -1;
	}

	string crlf("\r\n"), crlfcrlf("\r\n\r\n");
	size_t prev = 0, next = 0;
	size_t pos_crlfcrlf = 0;

	//prase requese line
	if ((next = data.find(crlf, prev)) != string::npos)
	{
		string reqline = data.substr(prev, next - prev);
		prev = next ;
		stringstream ss(reqline);
		ss  >> preq->method;
		ss >> preq->url;
		ss >> preq->version;
	}
	else
	{
		perror("parse_http_header  error: without \\r\\n");
		return -1;
	}

	if ((pos_crlfcrlf = data.find(crlfcrlf, prev)) == string::npos)
	{
		perror("parse_http_header error: without \\r\\n\\r\\n");
		return -1;
	}

	//prase request headerline
	string key, value;
	while(next != pos_crlfcrlf)
	{
		prev = next + 2;
		next = data.find(crlf, prev);
		stringstream stream(data.substr(prev, next - prev));
		stream >> key;
		stream >> value;
		preq->header.insert(pair<string, string>(key, value));
	}

	//prase request body
	prev = next + 4;
	preq->entitybody = data.substr(prev);

	return 0;
}

void db_print_header(ws_http_request *preq)
{
	cout << preq->method << " " << preq->url << " " << preq->version << "\r\n";
	for (map<string, string>::iterator iter = preq->header.begin(); iter != preq->header.end(); ++iter)
	{
		cout << iter->first << "  " << iter->second << "\r\n";
	}
	cout << "\r\n";
	cout << preq->entitybody;
}

string get_localtime()
{
	char loacltime[100];
	time_t ticks = time(NULL);
	int len = strlen(ctime(&ticks));
	strncpy(loacltime, ctime(&ticks), len-1);
	loacltime[len-1] = '\0';
	return string(loacltime);
}

void call_get(int sockfd, string url)
{
	url = string("webside") + url;
	string suffix, statusline, hdate, htype, hlength, resp;
	int fd, fsize;

	statusline = string(LOACL_HTTP_VERSION) + " " + to_string(WS_OK) + " " + ws_ok + WS_CRLF;
	hdate = string(WS_DATE_HEAD) + " " + get_localtime() + WS_CRLF;

	if ((fd = open(url.c_str(), O_RDONLY)) < 0)
	{
		url = "webside/404.html";
		fd = open(url.c_str(), O_RDONLY);
		statusline = string(LOACL_HTTP_VERSION) + " " + to_string(WS_NOTFOUND) + " " + ws_notfound + WS_CRLF;
	}
 	
 	struct stat statbuf;
 	stat(url.c_str(), &statbuf);
 	fsize = (int)statbuf.st_size;
 	hlength = string(WS_CONTENTLENGTH_HEAD) + " " + to_string(fsize) + WS_CRLF;

	suffix = url.substr(url.rfind('.'));
	if (suffix == ".html")
	{
		htype = string(WS_CONTENTTYPE_HEAD) + " " + "text/html" + WS_CRLF;
	}
	else if (suffix == ".ico")
	{
		htype = string(WS_CONTENTTYPE_HEAD) + " " + "image/ico" + WS_CRLF;
	}
	else
	{

	}
	resp = statusline + hdate + htype + hlength + WS_CRLF;

	const int csize = resp.size() + fsize + 20;

	char buf[csize];
	strncpy(buf, resp.c_str(), resp.size());
	int nread = read(fd, buf + resp.size(), csize);

	//send
	send(sockfd, buf, nread + resp.size(), 0);
}

void http_response(int sockfd, ws_http_request *preq)
{
	if (preq == NULL)
		perror("http request is null");
	if (preq->method == "GET")
		call_get(sockfd, preq->url);
}

#endif