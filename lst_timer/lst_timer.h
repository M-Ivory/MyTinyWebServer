//
// Created by ivory on 24-7-15.
//

#ifndef LST_TIMER_H
#define LST_TIMER_H

#include <ctime>
#include <netinet/in.h>

#include "../log/log.h"
class util_timer;

struct client_data {
    sockaddr_in address;
    int sockfd;// 与客户端通信的套接字文件描述符
    util_timer* timer;// 双向关联
};

class util_timer {// 定时器链表
public:
    util_timer():prev(NULL), next(NULL){}
    time_t expire;// 失效时间

    void (* cb_func)(client_data *user_data);// 函数指针 cb_func
    // cb_func 可以指向任何符合这个签名的函数，即接受一个 client_data * 参数并返回 void
    client_data* user_data;// 双向关联
    util_timer* prev;
    util_timer* next;
};

class sort_timer_list {
public:
    sort_timer_list():head(NULL), tail(NULL){}
    ~sort_timer_list();

    void add_timer(util_timer *timer);// 可用的加入定时器函数，因为head为私有变量
    void adjust_timer_list(util_timer *timer);// 调整list中序列不按expire升序排的
    void del_timer(util_timer *timer);// 删除定时器
    void tick();//
private:
    void add_timer(util_timer* timer, util_timer* list_head);
    util_timer* head;
    util_timer* tail;
};

class Utils {

};
#endif //LST_TIMER_H
