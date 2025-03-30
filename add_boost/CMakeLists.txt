cmake_minimum_required(VERSION 3.11)

project(Hello CXX)

find_package(Boost 1.87.0 REQUIRED)
if(Boost_FOUND)
  # Boost найден, добавляем к каталогам заголовочных файлов проекта путь к
  # заголовочным файлам Boost
  include_directories(${Boost_INCLUDE_DIRS})
endif()

# Платформы вроде linux требуют подключения библиотеки pthread для
# поддержки стандартных потоков.
# Следующие две строки подключат эту библиотеку на таких платформах
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(hello src/main.cpp)
# Просим компоновщик подключить библиотеку для поддержки потоков и boost
target_link_libraries(hello PRIVATE Threads::Threads boost::boost)
