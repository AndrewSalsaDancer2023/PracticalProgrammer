//#include "address.h"
#include <optional>
#include "basesocket.h"
#include "address.h"

#include <strings.h>
#include <string.h>
#include <iostream>

int buffer_size = 128;
int port = 8080;
std::string address{"127.0.0.1"};


void func(ClientSocket& sock)
{
    char buff[buffer_size];
    int n;
    for (;;) {
        bzero(buff, sizeof(buff));
        printf("Enter the string : ");
        n = 0;
        while ((buff[n++] = getchar()) != '\n')
            ;
        sock.write(buff, sizeof(buff));

        bzero(buff, sizeof(buff));
        sock.read(buff, sizeof(buff));

        printf("From Server : %s", buff);
        if ((strncmp(buff, "exit", 4)) == 0) {
            printf("Client Exit...\n");
            break;
        }
    }
}

int main() {
try {
	ClientSocket sock;
	sock.Connect(SocketAddress(address, port));
	func(sock);
}
catch(std::exception& ex)
{
    std::cout << ex.what() << std::endl;
}    
	return 0;
}
