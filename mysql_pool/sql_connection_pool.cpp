//
// Created by ivory on 24-7-11.
//
#include <mysql/mysql.h>
#include <string>
#include <iostream>
#include "sql_connection_pool.h"
#include "../log/log.h"
#include <string>
#include <list>
#include <pthread.h>

using namespace std;

connection_pool::connection_pool() {
    my_current_connections = 0;
    my_free_connections = 0;
}

connection_pool* connection_pool::get_instance() {
    static connection_pool newpool;
    return &newpool;
}

void connection_pool::init(string url, string user, string passwd, string db_name, int port, int max_connections, int close_log) {
    my_port = port;
    my_url = url;
    my_user = user;
    my_passwd = passwd;
    my_db_name = db_name;
    my_close_log = close_log;

    for(int i=0; i < max_connections; i++) {
        MYSQL* connection = NULL;
        connection = mysql_init(connection);
        if(connection == NULL) {
            LOG_ERROR("MySQL Error");// 此处调用宏使用的my_close_log为自己定义
            exit(1);
        }
        connection = mysql_real_connect(connection, url.c_str(), user.c_str(), passwd.c_str(), db_name.c_str(), port, NULL, 0);
        if(connection == NULL) {
            LOG_ERROR("MySQL Error");// 此处调用宏使用的my_close_log为自己定义
            exit(1);
        }
        connection_list.push_back(connection);
        my_free_connections++;
    }
    reserve = sem(my_free_connections);// 信号量，表示线程中资源可用个数，OS中的PV操作
    my_max_connections = my_free_connections;
}

// 从队列里取一个空闲的连接用
MYSQL* connection_pool::get_mysql_connection() {
    if(my_free_connections == 0)return NULL;
    MYSQL* connection = NULL;
    reserve.wait();// 对信号量-1操作(P操作)，可用资源-1
    my_lock.lock();
    connection = connection_list.front();
    connection_list.pop_front();
    ++my_current_connections;// 使用的connection+1
    --my_free_connections;// 空闲的connection-1
    my_lock.unlock();
    return connection;
}

// 释放一个用过的连接
bool connection_pool::release_connection(MYSQL *connection) {
    if(connection == NULL)return false;

    my_lock.lock();
    connection_list.push_back(connection);
    ++my_free_connections;
    --my_current_connections;
    reserve.post();// 对信号量+操作(V操作)，可用资源+1
    // 此处进入临界区后增加资源数量，才调整信号量，例如根据某种条件决定是否允许更多线程访问资源，我认为信号量应放在锁里面
    my_lock.unlock();

    // reserve.post();// 对信号量+操作(V操作)，可用资源+1
    return true;
}

// 空闲连接数
int connection_pool::get_free_connection_num() {
    return this->my_free_connections;
}

// 摧毁连接池
void connection_pool::destroy_pool() {
    my_lock.lock();
    if(connection_list.size() > 0) {
        for (list<MYSQL *>::iterator it = connection_list.begin(); it != connection_list.end(); ++it)// 使用迭代器，取list中每个元素
        {
            MYSQL *con = *it;
            mysql_close(con);
        }
        my_current_connections = 0;
        my_free_connections = 0;
        connection_list.clear();
    }
    my_lock.unlock();
}

connection_pool::~connection_pool() {
    destroy_pool();
}

connectionRAII::connectionRAII(MYSQL **con, connection_pool *connPool) {// 从pool中取了一个空闲连接
    // con为一个指向*MYSQL的指针，因为pool中存的都是*MYSQL
    *con = connPool->get_mysql_connection();// *con是MYSQL*类型的变量，*此处作用为解引用
    connRAII = *con;
    poolRAII = connPool;
}

connectionRAII::~connectionRAII() {// 释放用过的连接
    poolRAII->release_connection(connRAII);
}

