//
// Created by ivory on 24-7-15.
//
#include "../http/http_connection.h"
#include "lst_timer.h"

sort_timer_list::~sort_timer_list() {
    util_timer *tmp = head;
    while (tmp)
    {
        head = tmp->next;
        delete tmp;
        tmp = head;
    }
}

void sort_timer_list::add_timer(util_timer *timer) {
    if(!timer)return;
    if(!head) {
        head = tail = timer;
        return;
    }
    if(timer->expire < head->expire) {
        timer->next = head;
        head->prev = timer;
        head = timer;
        return;
    }
    // 之前排除了timer的expire小于head的expire的情况
    add_timer(timer, head);
}

void sort_timer_list::adjust_timer_list(util_timer *timer) {// 用于调整list中序列不按expire升序排的
    if(!timer)return;
    util_timer* tmp = timer->next;
    if(!tmp || timer->expire < tmp->expire)return;// 排序正确
    // assert：此时timer->expire >= timer->next->expire
    if(timer == head) {// 把timer先删除再add用于维持序列顺序
        head = head->next;
        head->prev = NULL;
        timer->next = NULL;
        add_timer(timer, head);
    }else {
        timer->prev->next = timer->next;
        timer->next->prev = timer->prev;
        add_timer(timer, timer->next);
    }
    // 此时使用两个参数的add_timer因为已经保证了timer的expire会大于参数二的expire
}

void sort_timer_list::del_timer(util_timer *timer) {// 删除某一个计时器
    if(!timer)return;
    if(timer == head && timer == tail) {
        delete timer;
        head = NULL;tail = NULL;
        return;
    }
    if(timer == head) {
        head = head->next;
        head->prev = NULL;
        delete timer;
        return;
    }
    if(timer == tail) {
        tail = tail->prev;
        tail->next = NULL;
        delete timer;
        return;
    }
    timer->prev->next = timer->next;
    timer->next->prev = timer->prev;
    delete timer;
}

// 处理链表中所有到期的定时任务
void sort_timer_list::tick() {
    if(!head)return;
    time_t cur = time(NULL);
    util_timer* tmp = head;
    while(tmp) {
        if(cur < tmp->expire)break;
        tmp->cb_func(tmp->user_data);// 释放tmp计时器
        head = tmp->next;
        if(head)head->prev = NULL;
        delete tmp;
        tmp = head;
    }
}

void sort_timer_list::add_timer(util_timer *timer, util_timer *list_head) {// 将timer按expire升序插入，前提为list_head的expire要小于timer的expire，因为调用的是public的add_timer
    util_timer* prev = list_head;
    util_timer* tmp = prev->next;
    while(tmp) {
        if(timer->expire < tmp->expire) {// 将timer按expire升序插入
            prev->next = timer;
            timer->next = tmp;
            tmp->prev = timer;
            timer->prev = prev;
            break;
        }
        prev = tmp;
        tmp = tmp->next;
    }
    if(!tmp) {
        prev->next = timer;
        timer->prev = prev;
        timer->next = NULL;
        tail = timer;
    }
}

void Utils::init(int timeslot) {
    MY_TIMESLOT = timeslot;
}

// 对文件描述符设置非阻塞
int Utils::set_non_blocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);// 获取文件描述符 fd 的当前状态标志；F_GETFL是获取/设置文件状态标志
    fcntl(fd, F_SETFL, old_option | O_NONBLOCK);// 在旧的状态标志基础上添加非阻塞标志，将文件描述符设为非阻塞模式
    return old_option;// 返回旧的文件状态标志，以便后来可以恢复；但此时fd已经被设为非阻塞模式了
}

// 将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
void Utils::add_fd(int epollfd, int fd, bool one_shot, int TRIGMode) {
    epoll_event event;
    event.data.fd = fd;
    if(TRIGMode == 1) event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    else event.events = EPOLLIN | EPOLLRDHUP;
    // EPOLLIN: 读事件。
    // EPOLLET: 边缘触发模式。
    // EPOLLRDHUP: 对端关闭连接。
    // 每一个宏定义占一位，用或运算，最后结果哪一位为1,则表示用了哪一位
    if(one_shot)event.events |= EPOLLONESHOT;
    // EPOLLONESHOT: 事件只触发一次，避免同一个文件描述符被多个线程处理
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    // 将文件描述符 fd 添加到 epoll 实例 epollfd 中，并注册事件
    set_non_blocking(fd);  // 设置文件描述符为非阻塞模式
}

// 信号处理函数
void Utils::sign_deal(int sign) {
    int save_errno = errno;// errno: 系统调用的错误码，保存当前errno
    int message = sign;
    send(u_pipefd[1], (char*)&message, 1, 0);// 将信号写入管道
    // 向套接字 FD 发送 N 字节的 BUF。 返回发送的字节数或-1。
    errno = save_errno;// 恢复errno
}

// 设置信号函数
void Utils::add_sign(int sign, void handler(int), bool restart) {// handler为函数
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));  // 初始化sigaction结构体
    sa.sa_handler = handler;  // 设置信号处理函数
    if (restart)
        sa.sa_flags |= SA_RESTART;  // 设置SA_RESTART标志，自动重启被中断的系统调用
    // 这个标志会自动重启被中断的系统调用
    sigfillset(&sa.sa_mask);  // 设置屏蔽字，屏蔽所有信号
    assert(sigaction(sign, &sa, NULL) != -1);  // 注册信号处理函数
}

void Utils::timer_handler() {
    my_timer_list.tick(); // 处理所有到期的定时任务
    alarm(MY_TIMESLOT);// 重新设置定时器以不断触发 SIGALRM 信号
}

void Utils::show_error(int connfd, const char *info) {
    send(connfd, info, strlen(info), 0);
    close(connfd);
}
// 此两静态数据初始化
int *Utils::u_pipefd = 0;
int Utils::u_epollfd = 0;

class Utils;

// 处理定时器到期时的回调函数，关闭连接并减少用户连接数。
void cb_func(client_data *user_data) {
    epoll_ctl(Utils::u_epollfd, EPOLL_CTL_DEL, user_data->sockfd, 0);
    // 从 epoll 实例 u_epollfd 中删除用户数据对应的文件描述符 user_data->sockfd
    assert(user_data);// 断言
    close(user_data->sockfd);// 关闭用户连接 user_data->sockfd
    http_connection::my_user_count--;// 减少用户连接数 m_user_count。这是一个静态成员变量，表示当前活跃的用户连接数。
}
