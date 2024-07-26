//
// Created by ivory on 24-7-15.
//

#ifndef LST_TIMER_H
#define LST_TIMER_H
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>

#include <time.h>
#include "../log/log.h"

class util_timer;

struct client_data {
    sockaddr_in address;
    int sockfd;// 与客户端通信的套接字文件描述符
    util_timer* timer;// 双向关联
};

class util_timer {// 定时器
public:
    util_timer():prev(NULL), next(NULL){}
    time_t expire;// 失效时间

    void (* cb_func)(client_data *user_data);// 函数指针 cb_func
    // cb_func 可以指向任何符合这个签名的函数，即接受一个 client_data * 参数并返回 void
    client_data* user_data;// 双向关联
    util_timer* prev;
    util_timer* next;
};

class sort_timer_list {// 定时器链表
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
public:
    Utils(){}
    ~Utils(){}

    void init(int timeslot);
    int set_non_blocking(int fd);// 将文件描述符设置为非阻塞模式
    void add_fd(int epollfd, int fd, bool one_shot, int TRIGMode);// 文件描述符 fd 添加到 epoll 实例中，注册读事件。
    static void sign_deal(int sign);// 信号处理函数，sign为收到的信号
    void add_sign(int sign, void(handler)(int), bool restart);// 设置信号函数
    void timer_handler();//定时处理任务，重新定时以不断触发SIGALRM信号
    void show_error(int connfd, const char *info);

public:
    static int* u_pipefd;// 进程间通信的管道文件描述符数组
    sort_timer_list my_timer_list;// 计时器列表
    static int u_epollfd;
    int MY_TIMESLOT;// 时间槽的大小
};


void cb_func(client_data* user_data);
#endif //LST_TIMER_H
