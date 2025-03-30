// Подключим библиотеку Boost.Format
#include <boost/format.hpp>
#include <iostream>

int main() {

    std::cout << boost::format("writing %1%,  version=%2% : date: %3%") % "Hello from boost! " % 1.87 % 30032025 << std::endl; 
    return 0;
}

