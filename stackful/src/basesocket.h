#pragma once
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <utility>
#include "address.h"

class BaseSocket {
public:
	BaseSocket(int domain, int type);
	BaseSocket(int fd);
	~BaseSocket(){close();}
	int GetFd() {return Fd_;}
	protected:
	int Setup(int s);
	int CreateSocket(int domain, int type);
	void close();
    int Fd_ = -1;	
};

class RWSocket: public BaseSocket {
public:
	RWSocket() : BaseSocket(AF_INET, SOCK_STREAM){}
	RWSocket(SocketAddress address, int fd);
    ssize_t read(void* buf, size_t count);
    ssize_t write(const void* buf, size_t count);
protected:    
	SocketAddress address_;
};

class BaseServerSocket: public BaseSocket {
public:
	BaseServerSocket(): BaseSocket(AF_INET, SOCK_STREAM) {}
    void Bind(SocketAddress address);
    void Listen(int backlog = 128);

    RWSocket Accept();    
private:
	SocketAddress GetAddress() { return address_;}
	SocketAddress address_;
};


class ClientSocket: public RWSocket {
public:
    void Connect(SocketAddress address);
private:
};
