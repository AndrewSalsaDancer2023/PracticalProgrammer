#pragma once
#include <string>
#include <utility>
#include <sys/socket.h>
#include "address.h"

int GetAcceptedClientDescriptor(int fd, sockaddr* addr, socklen_t len);
// int GetConnectionResult(int fd, SocketAddress address);
void ConnectToServer(int fd, SocketAddress& address);

/*
char clientaddr[sizeof(sockaddr_in6)];
                socklen_t len = static_cast<socklen_t>(sizeof(sockaddr_in6));

                int clientfd = accept(fd, reinterpret_cast<sockaddr*>(&clientaddr[0]), &len);
                if (clientfd < 0) {
                    throw std::system_error(errno, std::generic_category(), "accept");
                }

                return TSocket{TAddress{reinterpret_cast<sockaddr*>(&clientaddr[0]), len}, clientfd, *poller};
*/
/*
class IOEnginge {
public:
    IOEnginge(Poller);
    std::string ToString() const;
    std::pair<const sockaddr*, int> RawAddr();
private:
	sockaddr_in Addr_ = {};    
};
*/
