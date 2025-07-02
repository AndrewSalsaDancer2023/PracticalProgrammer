#include "basesocket.h"
#include <cstring>
#include  <stdexcept>
#include <unistd.h>
#include <utility>
#include "io_util.h"
#include <stdexcept>
#include <system_error>

//* - IPv4: "127.0.0.1:8080"
//int buffer_size = 128;
//TAddress address{"::1", port};
//run<TEPoll>(debug, address, buffer_size);
/*
Poller_.Poll();
        Poller_.WakeupReadyHandles();
*/
BaseSocket::BaseSocket(int domain, int type)
{
	Fd_ = CreateSocket(domain, type);
}

BaseSocket::BaseSocket(int fd)
{
	// Fd_ = Setup(fd); TODO
    Fd_ = fd;
}

int BaseSocket::Setup(int s)
{
    int value;
    socklen_t len = sizeof(value);
    if (getsockopt(s, SOL_SOCKET, SO_TYPE, (char*) &value, &len) == 0) {
        value = 1;
        if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (char*) &value, len) < 0) {
                throw std::system_error(errno, std::generic_category(), "setsockopt");
        }
    }

    auto flags = fcntl(s, F_GETFL, 0);
    if (flags < 0) {
        throw std::system_error(errno, std::generic_category(), "fcntl");
    }
    if (fcntl(s, F_SETFL, flags | O_NONBLOCK) < 0) {
        throw std::system_error(errno, std::generic_category(), "fcntl");
    }
    return s;
}

int BaseSocket::CreateSocket(int domain, int type)
{
    auto s = socket(domain, type, 0);
    if (s == static_cast<decltype(s)>(-1)) {
        throw std::system_error(errno, std::generic_category(), "socket");
    }
   // return Setup(s); TODO
   return s;
}

void BaseSocket::close()
{
	if (Fd_ >= 0) {
		::close(Fd_);
    }
}

void BaseServerSocket::Bind(SocketAddress address)
{
	address_ = std::move(address);
	auto [rawaddr, len] = address_.RawAddr();
    int optval = 1;
    socklen_t optlen = sizeof(optval);
    if (setsockopt(Fd_, SOL_SOCKET, SO_REUSEADDR, (char*) &optval, optlen) < 0) {
        throw std::system_error(errno, std::generic_category(), "setsockopt");
    }
    if (bind(Fd_, rawaddr, len) < 0) {
        throw std::system_error(errno, std::generic_category(), "bind");
    }
	
}

void BaseServerSocket::Listen(int backlog)
{
    if (listen(Fd_, backlog) < 0) {
        throw std::system_error(errno, std::generic_category(), "listen");
    }
}
 
RWSocket BaseServerSocket::Accept()
{
    char clientaddr[sizeof(sockaddr_in)];
    socklen_t len = static_cast<socklen_t>(sizeof(sockaddr_in));
    
	int clientfd = GetAcceptedClientDescriptor(GetFd(),reinterpret_cast<sockaddr*>(&clientaddr[0]), len);
	if (clientfd < 0) {
		throw std::system_error(errno, std::generic_category(), "accept");
	}

//    char clientaddr[sizeof(sockaddr_in)];
//    socklen_t len = static_cast<socklen_t>(sizeof(sockaddr_in));
   return RWSocket(SocketAddress{reinterpret_cast<sockaddr*>(&clientaddr[0])}, clientfd);
}
 
RWSocket::RWSocket(SocketAddress address, int fd)
			:BaseSocket(fd)
{
	address_ = std::move(address);
}

ssize_t RWSocket::read(void* buf, size_t count)
{
	return ::read(Fd_, buf, count); 
}

ssize_t RWSocket::write(const void* buf, size_t count)
{
	return ::write(Fd_, buf, count);
}


void ClientSocket::Connect(SocketAddress address)
{
	address_ = std::move(address);
	//address.RawAddr()
	
	// int ret = GetConnectionResult(GetFd(), address);
    ConnectToServer(GetFd(), address_);
//	int ret = poller->Result();
    // if (ret < 0) {
	// 	throw std::system_error(-ret, std::generic_category(), "connect");
	// }
}
