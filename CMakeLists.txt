cmake_minimum_required(VERSION 3.28)
project(MyTinyWebServer)

set(CMAKE_CXX_STANDARD 17)

add_executable(MyTinyWebServer main.cpp
        Config.cpp
        Config.h
        lock/lockDefine.h
        log/block_queue.h)
