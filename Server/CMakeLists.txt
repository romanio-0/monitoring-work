cmake_minimum_required(VERSION 3.26)
project(Server)

set(CMAKE_CXX_STANDARD 17)

set(FILE_CPP main.cpp image.cpp sup.cpp)

add_executable(Server ${FILE_CPP})

target_link_libraries(Server C:/Windows/System32/Ws2_32.dll)
target_link_libraries(Server C:/Windows/System32/Gdiplus.dll)
