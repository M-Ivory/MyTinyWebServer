cmake_minimum_required(VERSION 3.28)
project(MyTinyWebServer)

set(CMAKE_CXX_STANDARD 17)

add_executable(MyTinyWebServer main.cpp
        Config.cpp
        Config.h
        lock/lockDefine.h
        log/block_queue.h
        log/log.cpp
        log/log.h
        mysql_pool/sql_connection_pool.cpp
        mysql_pool/sql_connection_pool.h
        threadpool/threadpool.h
        threadpool/threadpool.cpp)
