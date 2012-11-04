
#ifndef __SOCKET_H
#define __SOCKET_H

#include <string>
#include <vector>

class Socket {
private:
	struct Core;
	Core *core;
	void assign(Core *p);
public:
	Socket();

	Socket(Socket const &r)
		: core(0)
	{
		assign(r.core);
	}

	~Socket()
	{
		assign(0);
	}

	void operator = (Socket const &r)
	{
		assign(r.core);
	}
private:
	void parse_http_result(
		std::vector<std::string>& resheader, 
		std::vector<unsigned char>& resdata);
public:
	void socket(int af, int type, int protocol);
	void connect(const char *name, int port);
	void http_get(const char *uri, 
		std::vector<std::string>& header, 
		std::vector<std::string>& resheader, 
		std::vector<unsigned char>& resdata);
	void http_post(const char *uri, 
		std::vector<std::string>& header, 
		const void *content, size_t contentlen,
		const char *contet_type,
		std::vector<std::string>& resheader, 
		std::vector<unsigned char>& resdata);
	void close();
	void clear();

	int recv(void *buf, size_t len);
	int send(const void *buf, size_t len);

	int read(std::vector<unsigned char> *buffer);
	int write(const void *data, int datalen);
	int write(const char *data);

	static void initialize();
	static void finalize();
};

#endif
