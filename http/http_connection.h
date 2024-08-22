//
// Created by ivory on 24-7-14.
//

#ifndef HTTP_CONNECTION_H
#define HTTP_CONNECTION_H

#include <netinet/in.h>
#include <sys/stat.h>
#include <map>
#include <sys/uio.h>
#include <sys/epoll.h>
#include <unistd.h>
#include "../lock/lockDefine.h"
#include "../mysql_pool/sql_connection_pool.h"
// #include "../timer/lst_timer.h"
#include "../log/log.h"

class http_connection {
public:
    static const int FILENAME_LENGTH = 200;// 文件最大长度
    static const int READ_BUFF_SIZE = 2048;// 读缓冲区
    static const int WRITE_BUFF_SIZE = 1024;// 写缓冲区
    enum METHOD{GET, POST, HEAD, PUT, DELETE, TRACE, OPTIONS, CONNECT, PATH};//http请求类型
    enum CHECK_STATE{REQUEST_LINE, HEADER, CONTENT};// 主状态机的状态
    enum HTTP_STATE{NO_REQUEST, GET_REQUEST, BAD_REQUEST, NO_RESOURCE,
        FORBIDDEN_REQUEST, FILE_REQUEST, INTERNAL_ERROR, CLOSED_CONNECTION};// 连接状态，例如200、404
    enum LINE_STATUS{LINE_OK, LINE_BAD, LINE_OPEN};// 读取行状态
public:
    http_connection()= default;
    ~http_connection()= default;
public:
    void init(int sockfd, const sockaddr_in &addr, char *, int, int, string user, string passwd, string sqlname);
    void close_conn(bool real_close = true);
    void process();
    bool read_once();
    bool write();
    sockaddr_in *get_address()
    {
        return &my_address;
    }
    void initmysql_result(connection_pool *connPool);
    int timer_flag;
    int improv;
private:
    void init();
    HTTP_STATE process_read();
    bool process_write(HTTP_STATE ret);
    HTTP_STATE parse_request_line(char *text);
    HTTP_STATE parse_headers(char *text);
    HTTP_STATE parse_content(char *text);
    HTTP_STATE do_request();
    char *get_line() { return my_read_buf + my_start_line; };
    LINE_STATUS parse_line();
    void unmap();
    bool add_response(const char *format, ...);
    bool add_content(const char *content);
    bool add_status_line(int status, const char *title);
    bool add_headers(int content_length);
    bool add_content_type();
    bool add_content_length(int content_length);
    bool add_linger();
    bool add_blank_line();

public:
    static int my_epollfd;// epoll描述符
    static int my_user_count;// 当前连接数
    MYSQL* mysql;// 数据库连接
    int my_state;// 事件类型，0为读，1为写

private:
    int my_sockfd;// 客户端套接字文件描述符
    // 如：IP：Port
    sockaddr_in my_address;// 表示 IPv4 网络地址
    char my_read_buf[READ_BUFF_SIZE];// 读缓冲区，用于存放客户端请求数据
    long my_read_buf_index;// 读缓冲的指示器
    char my_write_buf[WRITE_BUFF_SIZE];// 写缓冲区，用于存放服务器响应数据。
    long my_write_buf_index;// 读缓冲的指示器
    long my_checked_idx;// 当前解析位置的下标
    int my_start_line;// 每行数据的起始位置
    CHECK_STATE my_check_state;// 当前主状态机的状态
    METHOD my_method;// 当前http请求方法
    char my_real_file[FILENAME_LENGTH];// 真实文件名是去除路径的文件名or绝对路径文件名？
    char *my_url;// 请求目标url
    char *my_version;// http版本
    char *my_host;// host字段
    long my_content_length;// 内容长度
    bool my_linger;// 表示http的连接状态 Keep-Alive可持续连接
    char *my_file_address;// 文件的内存映射地址
    struct stat my_file_stat;// 请求文件的状态信息
    struct iovec my_iv[2];// 向量 I/O 操作中的缓冲区
    // 可以分散读和集中写
    // 向量 I/O 操作是一种高效的数据传输方式，可以将多个缓冲区的数据一次性地进行读写操作
    // iov_base：指向缓冲区的起始地址。
    // iov_len：表示缓冲区的长度。
    int my_iv_count;// ivoec结构体的数量
    int cgi;// 表示是否启用了CGI
    char *my_string; // 存储post请求的请求体数据
    int bytes_to_send;// 要发送的字节数
    int bytes_have_send;// 已经发送的字节数
    char *doc_root;// 服务器根目录

    map<string, string> my_users;// 用户名和密码的数据库映射
    int my_trig_mode;// 触发器模式，分LT和ET
    int my_close_log;// 日志开关

    char sql_user[100];// 数据库用户名
    char sql_passwd[100];// 数据库密码
    char sql_name[100];// 数据库名
};

#endif //HTTP_CONNECTION_H
