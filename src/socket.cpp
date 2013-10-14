
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#include <winsock.h>
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netdb.h>
#include <unistd.h>
#endif

#include "socket.h"

#ifdef WIN32
typedef SOCKET socket_t;
#pragma warning(disable:4996)
#define sleep_ms(ms) Sleep(ms)
#define isblocking() (WSAGetLastError() == WSAEWOULDBLOCK)
#else
typedef int socket_t;
#define INVALID_SOCKET (-1)
#define closesocket(FD) close(FD)
#define sleep_ms(ms) usleep(ms * 1000)
#define isblocking() (WSAGetLastError() == WSAEWOULDBLOCK)
#endif

const int SOCKET_TIME_OUT = 1000;

#define USER_AGENT "tinytwit"

void Socket::initialize()
{
#ifdef WIN32
	WSADATA wsaData;
	WORD wVersionRequested;
	wVersionRequested = MAKEWORD(1,1);
	WSAStartup(wVersionRequested, &wsaData);
#endif
}

void Socket::finalize()
{
#ifdef WIN32
	WSACleanup();
#endif
}

struct Socket::Core {
	unsigned int ref;
	socket_t sock;
	Core()
		: ref(0)
		, sock(INVALID_SOCKET)
	{
	}
};

Socket::Socket()
	: core(0)
{
	assign(new Core());
}

void Socket::assign(Core *p)
{
	if (p) {
		p->ref++;
	}
	if (core) {
		if (core->ref > 1) {
			core->ref--;
		} else {
			delete core;
		}
	}
	core = p;
}

void Socket::socket(int af, int type, int protocol)
{
	assign(new Core());
	core->sock = ::socket(af, type, protocol);
}

struct http_request_t {
	std::string name;
	int port;
	std::string query;
	http_request_t()
	{
		port = 0;
	}
};

static http_request_t parse_http(const char *uri)
{
	if (strncmp(uri, "http://", 7) != 0) {
		return http_request_t();
	}

	http_request_t x;

	char const *p;
	p = strchr(uri + 7, '/');
	if (p) {
		x.query = p;
	} else {
		p = uri + strlen(uri);
		x.query = "/";
	}
	x.name = std::string(uri + 7, p);
	x.port = 80;
	p = strchr(x.name.c_str(), ':');
	if (p) {
		x.port = atoi(p + 1);
		x.name = std::string(x.name.c_str(), p);
	}

	return x;
}

void Socket::connect(const char *name, int port)
{
	struct hostent *host;
	struct sockaddr_in addr;

	host = gethostbyname(name);
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	memcpy(&addr.sin_addr, host->h_addr, host->h_length);
	addr.sin_port = htons(port);
	::connect(core->sock, (struct sockaddr *)&addr, sizeof(sockaddr_in));
	
	u_long val = 1;
	ioctlsocket(core->sock, FIONBIO, &val);
}

void Socket::parse_http_result(
	std::vector<std::string>& resheader, 
	std::vector<unsigned char>& resdata)
{
	char buffer[4096];
	bool headerEnd = false;
	size_t datacnt = 0, datalen = 0xffffffff;
	size_t bufcnt = 0;
	while (!headerEnd || datacnt < datalen) {
		int size = this->recv(buffer + bufcnt, sizeof(buffer) - bufcnt);
		if (size <= 0) {
			if (WSAGetLastError() == WSAEWOULDBLOCK) {
				Sleep(1);
				continue;
			} else {
				break;
			}
		}
		char* ptr = buffer;
		bufcnt += size;
		if (!headerEnd) {
			while (1) {
				char* r = (char*)memchr(ptr, '\r', bufcnt);
				if (r == NULL) {
					break;
				}
				size_t len = (size_t)(r - ptr);
				if (len < bufcnt && r[1] == '\n') {
					if (ptr == r) {
						ptr += 2;
						bufcnt -= 2;
						headerEnd = true;
						break;
					}
					r[0] = '\0';
					resheader.push_back(std::string(ptr));
					if (memcmp(ptr, "Content-Length: ", 16) == 0 || 
						memcmp(ptr, "content-length: ", 16) == 0
					) {
						datalen = (size_t)atoi(&ptr[16]);
						resdata.resize(datalen);
					}
					len += 2;
					ptr += len;
					bufcnt -= len;
				} else {
					break;
				}
			}
		}
		if (headerEnd) {
			if (datalen == 0xffffffff) {
				break;
			}
			size_t len = min(bufcnt, datalen - datacnt);
			memcpy(&resdata[datacnt], ptr, len);
			datacnt += len;
			ptr += len;
			bufcnt -= len;
		}
	}
}

void Socket::http_get(const char *uri, 
	std::vector<std::string>& header, 
	std::vector<std::string>& resheader, 
	std::vector<unsigned char>& resdata)
{
	resheader.clear();
	resdata.clear();

	if (strncmp(uri, "http://", 7) != 0) {
		return;
	}

	http_request_t http = parse_http(uri);

	socket(AF_INET, SOCK_STREAM, 0);
	connect(http.name.c_str(), http.port);

	std::string req;
	req  = std::string("GET ") + http.query + " HTTP/1.1\r\n";
	req += std::string("User-Agent: ") + USER_AGENT + "\r\n";
	req += std::string("Host: ") + http.name + "\r\n";

	for (std::vector<std::string>::iterator it = header.begin();
		it != header.end(); it++
	) {
		req += *it;
		req += "\r\n";
	}
	req += "\r\n";

	write(req.c_str(), req.size());
	parse_http_result(resheader, resdata);
}

static inline std::string to_s(size_t n)
{
	char tmp[100];
	sprintf(tmp, "%u", n);
	return tmp;
}


void Socket::http_post(const char *uri, 
	std::vector<std::string>& header, 
	const void *content, size_t contentlen,
	const char *contet_type,
	std::vector<std::string>& resheader, 
	std::vector<unsigned char>& resdata)
{
	resheader.clear();
	resdata.clear();

	if (strncmp(uri, "http://", 7) != 0) {
		return;
	}

	http_request_t http = parse_http(uri);

	socket(AF_INET, SOCK_STREAM, 0);
	connect(http.name.c_str(), http.port);

	std::string req;
	req  = std::string("POST ") + http.query + " HTTP/1.1\r\n";
	req += std::string("User-Agent: ") + USER_AGENT + "\r\n";
	req += std::string("Host: ") + http.name + "\r\n";
	req += std::string("Content-Length: ") + to_s(contentlen) + "\r\n";

	if (contet_type) {
		req += std::string("Content-Type: ") + contet_type + "\r\n";
	} else {
		req += std::string("Content-Type: application/x-www-form-urlencoded\r\n");
	}

	for (std::vector<std::string>::iterator it = header.begin(); 
		it != header.end(); it++
	) {
		req += *it;
		req += "\r\n";
	}
	req += "\r\n";
	//OutputDebugStringA(req.c_str());
	write(req.c_str(), req.size());
	write(content, contentlen);
	parse_http_result(resheader, resdata);
	//OutputDebugStringA((const char*)&resdata[0]);
}

void Socket::close()
{
	::closesocket(core->sock);
}

void Socket::clear()
{
	core->sock = INVALID_SOCKET;
}

int Socket::recv(void *buf, size_t len)
{
	return ::recv(core->sock, (char *)buf, len, 0);
}

int Socket::send(const void *buf, size_t len)
{
	return ::send(core->sock, (char const *)buf, len, 0);
}

int Socket::read(std::vector<unsigned char> *buffer)
{
	buffer->clear();

	int count = 0;
	char tmp[4096];
	while (1) {
		int n = recv(tmp, sizeof(tmp));
		if (n < 1) {
			if (isblocking()) {
				if (++count >= SOCKET_TIME_OUT) {
					break;
				}
				sleep_ms(1);
			} else {
				break;
			}
		} else {
			count = 0;
			buffer->insert(buffer->end(), tmp, tmp + n);
		}
	}
	return count;
}

int Socket::write(const void *data, int datalen)
{
	int count = 0;
	while (datalen > 0) {
		int n = send(data, datalen);
		if (n < 1 || n > datalen) {
			if (isblocking()) {
				if (++count >= SOCKET_TIME_OUT) {
					break;
				}
				sleep_ms(1);
			} else {
				break;
			}
		}
		datalen -= n;
		data = (char*)data + n;
	}
	return count;
}

int Socket::write(const char *data)
{
	return write(data, strlen(data));
}
