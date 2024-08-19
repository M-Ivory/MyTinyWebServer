//
// Created by ivory on 24-7-14.
//

#include "http_connection.h"

#include <cstring>
#include <fcntl.h>
#include <mysql/mysql.h>
#include <fstream>

//定义http响应的一些状态信息
const char *ok_200_title = "OK";
const char *error_400_title = "Bad Request";
const char *error_400_form = "Your request has bad syntax or is inherently impossible to staisfy.\n";
const char *error_403_title = "Forbidden";
const char *error_403_form = "You do not have permission to get file form this server.\n";
const char *error_404_title = "Not Found";
const char *error_404_form = "The requested file was not found on this server.\n";
const char *error_500_title = "Internal Error";
const char *error_500_form = "There was an unusual problem serving the request file.\n";

locker my_lock;// 互斥锁
map<string, string> users;// sql用户名密码

// 查询所有用户名和密码映射并存在全局变量users中
void http_connection::initmysql_result(connection_pool *connPool) {
    MYSQL* mysql = NULL;
    connectionRAII mysqlcon(&mysql, connPool);// 创建一个sql连接类，这个连接销毁时自动释放
    if(mysql_query(mysql, "SELECT username,passwd FROM user"))// 执行一条 MySQL 查询
        LOG_ERROR("SELECT error:%s\n", mysql_error(mysql));
    MYSQL_RES* result = mysql_store_result(mysql);// 查询结果
    int num_fields = mysql_num_fields(result);// 查询结果列数
    MYSQL_FIELD* fields = mysql_fetch_field(result);// 查询结果结构数组
    while(MYSQL_ROW row = mysql_fetch_row(result)) {
        // row为查询结果中每一行的数据
        string temp1(row[0]);
        string temp2(row[1]);
        users[temp1] = temp2;// 存储用户名密码的映射

    }
}

// 对文件描述符设置非阻塞
// 可见lst_timer.cpp
int setnonblocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

// 添加文件描述符到epoll中
void addfd(int epollfd, int fd, bool one_shot, int TRIGMode) {
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
    setnonblocking(fd);  // 设置文件描述符为非阻塞模式
}

// 从epoll中移除文件描述符
void removefd(int epollfd, int fd)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

// 修改epoll中文件描述符的事件
void modfd(int epollfd, int fd, int ev, int TRIGMode) {
    // ev为新的事件类型，可能为EPOLLIN、EPOLLOUT等
    epoll_event event;
    event.data.fd = fd;
    if(TRIGMode == 1) event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    else event.events = ev | EPOLLONESHOT | EPOLLRDHUP;
    // EPOLLIN: 读事件。
    // EPOLLET: 边缘触发模式。
    // EPOLLRDHUP: 对端关闭连接。
    // 每一个宏定义占一位，用或运算，最后结果哪一位为1,则表示用了哪一位
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
    // EPOLL_CTL_MOD: 修改已注册的文件描述符事件
}

int http_connection::my_user_count = 0;
int http_connection::my_epollfd = -1;

// 初始化连接
void http_connection::init(int sockfd, const sockaddr_in &addr, char *root,
    int TRIGMode, int close_log, string user, string passwd, string sqlname) {
    my_sockfd = sockfd;// 客户端socket套接字
    my_address = addr;// 客户端地址
    addfd(my_epollfd, sockfd, true, my_trig_mode);// 将客户端socket添加到epoll实例中
    my_user_count++;// 连接数+1
    doc_root = root;// 设置服务器根目录
    strcpy(sql_user, user.c_str());
    strcpy(sql_passwd, passwd.c_str());
    strcpy(sql_name, sqlname.c_str());
    init();
}

// 关闭连接，关闭一个连接，客户总量减一
void http_connection::close_conn(bool real_close) {
    if(real_close && my_sockfd != -1) {
        printf("close %d\n", my_sockfd);
        removefd(my_epollfd, my_sockfd);
        my_sockfd = -1;
        my_user_count--;
    }
}

void http_connection::process() {
}

bool http_connection::read_once() {
}

bool http_connection::write() {
}



void http_connection::init() {
}

http_connection::HTTP_STATE http_connection::process_read() {
}

bool http_connection::process_write(HTTP_STATE ret) {
}

http_connection::HTTP_STATE http_connection::parse_request_line(char *text) {
}

http_connection::HTTP_STATE http_connection::parse_headers(char *text) {
}

http_connection::HTTP_STATE http_connection::parse_content(char *text) {
}

http_connection::HTTP_STATE http_connection::do_request() {
}

http_connection::LINE_STATUS http_connection::parse_line() {
}

void http_connection::unmap() {
}

bool http_connection::add_response(const char *format, ...) {
}

bool http_connection::add_content(const char *content) {
}

bool http_connection::add_status_line(int status, const char *title) {
}

bool http_connection::add_headers(int content_length) {
}

bool http_connection::add_content_type() {
}

bool http_connection::add_content_length(int content_length) {
}

bool http_connection::add_linger() {
}

bool http_connection::add_blank_line() {
}
