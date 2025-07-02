#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h>

#include <stdexcept>
#include <system_error>
#include <string>
#include "io_util.h"

int GetAcceptedClientDescriptor(int fd, sockaddr* addr, socklen_t len)
{
    if(!addr)
        throw std::runtime_error("GetAcceptedClientDescriptor: addr is null");
//    char clientaddr[sizeof(sockaddr_in)];
//    socklen_t len = static_cast<socklen_t>(sizeof(sockaddr_in));

//    int clientfd = accept(fd, reinterpret_cast<sockaddr*>(&clientaddr[0]), &len);
    int clientfd = accept(fd, addr, &len);
    if (clientfd < 0) {
        throw std::system_error(errno, std::generic_category(), "accept");
    }

	return clientfd;
    //return 0;
}


// int GetConnectionResult(int fd, SocketAddress address)
void ConnectToServer(int fd, SocketAddress& address)
{
    auto [rawaddr, len] = address.RawAddr();
    if (connect(fd, rawaddr, len) != 0) 
        throw std::system_error(errno, std::generic_category(), "connect");        

    // return 0;
}

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
