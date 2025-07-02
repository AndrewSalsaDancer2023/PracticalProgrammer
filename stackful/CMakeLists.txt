cmake_minimum_required(VERSION 3.15)

set(ProjectName main)
project(${ProjectName} CXX)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(server
		src/main.cpp
		src/address.cpp
		src/io_util.cpp
		src/basesocket.cpp		

)

add_executable(client
		src/main_client.cpp
		src/address.cpp
		src/io_util.cpp
		src/basesocket.cpp		

)