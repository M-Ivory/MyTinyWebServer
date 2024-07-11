//
// Created by ivory on 24-7-11.
//

#ifndef SQL_CONNECTION_POOL_H
#define SQL_CONNECTION_POOL_H
#include <mysql/mysql.h>
#include <list>
#include "../lock/lockDefine.h"
#include "../log/log.h"

using namespace std;

class connection_pool {
public:
    MYSQL* get_mysql_connection();// 获取数据库连接
    bool release_connection(MYSQL* connection);// 释放连接
    int get_free_connection_num();// 获取当前空闲连接数
    void destroy_pool();// 销毁所有连接
    //单例模式
    // 使用static来获取一个connection_pool的原因是，使得只有一个连接池例子
    static connection_pool *get_instance();

    void init(string url, string user, string passwd, string db_name, int port, int max_connections, int close_log);
private:
    
    connection_pool();
    ~connection_pool();
    int my_max_connections;// 最大连接数
    int my_current_connections;// 当前连接数
    int my_free_connections;// 当前空闲连接数
    locker my_lock;// 互斥锁
    list<MYSQL *> connection_list;// 连接池，用list表示
    // 连接池中，加入但未使用的、可使用的MYSQL数为free，以使用的MYSQL数为current，总和为max
    sem reserve;// 信号量
    
public:
    string my_url;// 主机地址
    int my_port;// 数据库端口号
    string my_user;// 登陆数据库用户名
    string my_passwd;// 登陆数据库密码
    string my_db_name;// 使用数据库名
    int my_close_log;// 日志开关
};

class connectionRAII {// 使用 RAII（Resource Acquisition Is Initialization）的编程惯用法来确保连接资源的正确获取和释放
    // 若使用 connectionRAII connection(&con, pool); 语句来获取和管理连接
    // 这个语句用构造函数构造了一个名为connection的connectionRAII类
    // 在这里con为*MYSQL，pool为list<MYSQL *>，进行数据库操作
    // 不需要显式释放连接，当 connectionRAII 对象销毁时连接会自动释放
    // 即用这种方法从池中取得连接，使用完connectionRAII类后，销毁时会自己调用析构函数释放连接
public:
    connectionRAII(MYSQL **con, connection_pool *connPool);
    ~connectionRAII();
private:
    MYSQL* connRAII;
    connection_pool* poolRAII;
};

#endif //SQL_CONNECTION_POOL_H
