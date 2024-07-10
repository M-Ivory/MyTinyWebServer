//
// Created by ivory on 24-7-11.
//

#ifndef SQL_CONNECTION_POOL_H
#define SQL_CONNECTION_POOL_H
#include <mysql/mysql.h>
#include "../lock/lockDefine.h"
#include "../log/log.h"

using namespace std;

class connection_pool {
private:
    connection_pool();
    ~connection_pool();
};
#endif //SQL_CONNECTION_POOL_H
