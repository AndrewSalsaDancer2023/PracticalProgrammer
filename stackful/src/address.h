#pragma once
#include <string>
#include <utility>
#include <sys/socket.h>

#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
/**
     * @brief Gets the domain (address family) of the stored address.
     *
     * Returns AF_INET for IPv4 or AF_INET6 for IPv6.
     *
     * @return The address family (e.g., AF_INET or AF_INET6).
     */
/*
class TAddress {
public:
    TAddress(const std::string& addr, int port);
    TAddress(sockaddr* addr, socklen_t len);

    std::pair<const sockaddr*, int> RawAddr() const;
    int Domain() const;
    std::string ToString() const;        
private:
	sockaddr_in Addr_ = {};
};
*/


class SocketAddress {
public:
	SocketAddress()=default;
    SocketAddress(const std::string& addr, int port);
    SocketAddress(sockaddr* addr);
    std::string ToString() const;
    std::pair<const sockaddr*, int> RawAddr() const;
private:
	struct sockaddr_in Addr_ = {};    
};
