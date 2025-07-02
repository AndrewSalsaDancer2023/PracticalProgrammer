//#include "address.h"
#include <optional>
#include "basesocket.h"
#include "address.h"
#include <iostream>
#include <vector>

int buffer_size = 128;
int port = 8080;
std::string address{"127.0.0.1"};


void client_handler(RWSocket& socket, int buffer_size) {
    std::vector<char> buffer(buffer_size); 

    try {
        for( ; ;)
        {
            int size = socket.read(buffer.data(), buffer_size);
            if(size > 0)
            {
               std::string recv(buffer.data());
               //std::cout << "Received: " << std::string_view(buffer.data(), size) << "\n";
               std::cout << "Received: " << recv << "\n";
            
              socket.write(buffer.data(), size);
              if(recv == "\n")
              {
                std::cout << "Exit" << std::endl;
                return;
              }
            }
            else
            {
                std::error_condition econd = std::generic_category().default_error_condition(EDOM);
                std::cout << "Category: " << econd.category().name() << '\n'
                            << "Value: " << econd.value() << '\n'
                            << "Message: " << econd.message() << '\n';
            }
        }
        
    } catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << "\n";
    }
    /*if (size == 0) {
        std::cerr << "Client disconnected\n";
    }*/
}



int main() {
try {
	BaseServerSocket sock;
	sock.Bind(SocketAddress(address, port));
	sock.Listen();
	while(true) {
		auto clientSocket = sock.Accept();
		client_handler(clientSocket, buffer_size);
	}
}
catch(std::exception& ex)
{
    std::cout << ex.what() << std::endl;
}
	return 0;
}


