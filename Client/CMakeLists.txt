cmake_minimum_required(VERSION 3.26)
project(Client)

set(CMAKE_CXX_STANDARD 17)

set(FILE_CPP ./main.cpp
        autorun.cpp
        autorun.h)

add_executable(Client ${FILE_CPP})

target_link_libraries(Client C:/Windows/System32/Ws2_32.dll)
target_link_libraries(Client C:/Windows/System32/Gdiplus.dll)




