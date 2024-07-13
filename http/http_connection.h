//
// Created by ivory on 24-7-14.
//

#ifndef HTTP_CONNECTION_H
#define HTTP_CONNECTION_H

#include <netinet/in.h>
#include <sys/stat.h>
#include <map>
#include <sys/uio.h>
#include "../lock/lockDefine.h"
#include "../mysql_pool/sql_connection_pool.h"
// #include "../timer/lst_timer.h"
#include "../log/log.h"

class http_connection {
public:
    static const int FILENAME_LENGTH = 200;
    static const int READ_BUFF_SIZE = 2048;
    static const int WRITE_BUFF_SIZE = 1024;
    enum METHOD{GET, POST, HEAD, PUT, DELETE, TRACE, OPTIONS, CONNECT, PATH};
    enum CHECK_STATE{REQUEST_LINE, HEADER, CONTENT};
    enum HTTP_STATE{NO_REQUEST, GET_REQUEST, BAD_REQUEST, NO_RESOURCE,
        FORBIDDEN_REQUEST, FILE_REQUEST, INTERNAL_ERROR, CLOSED_CONNECTION};// 连接状态，例如200、404
    enum LINE_STATUS{LINE_OK, LINE_BAD, LINE_OPEN};
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
    static int my_epollfd;//
    static int my_user_count;
    MYSQL* mysql;
    int my_state;// 事件类型，0为读，1为写

private:
    int my_sockfd;// 
    sockaddr_in my_address;// 表示 IPv4 网络地址
    char my_read_buf[READ_BUFF_SIZE];
    long my_read_buf_index;// 读缓冲的指示器
    char my_write_buf[WRITE_BUFF_SIZE];
    long my_write_buf_index;// 读缓冲的指示器
    long my_checked_idx;// 
    int my_start_line;//
    CHECK_STATE my_check_state;// 
    METHOD my_method;//
    char my_real_file[FILENAME_LENGTH];// 真实文件名是去除路径的文件名or绝对路径文件名？
    char *my_url;//
    char *my_version;//
    char *my_host;//
    long my_content_length;// 内容长度
    bool my_linger;//
    char *my_file_address;// 文件路径
    struct stat my_file_stat;// 获取文件的状态信息
    struct iovec my_iv[2];// 向量 I/O 操作中的缓冲区
    // 向量 I/O 操作是一种高效的数据传输方式，可以将多个缓冲区的数据一次性地进行读写操作
    // iov_base：指向缓冲区的起始地址。
    // iov_len：表示缓冲区的长度。
    int my_iv_count;//
    int cgi;        //是否启用的POST
    char *my_string; //存储请求头数据
    int bytes_to_send;//
    int bytes_have_send;//
    char *doc_root;

    map<string, string> my_users;
    int my_trig_mode;// 触发器模式
    int my_close_log;// 日志开关

    char sql_user[100];
    char sql_passwd[100];
    char sql_name[100];
};

#endif //HTTP_CONNECTION_H
