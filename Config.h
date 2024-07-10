//
// Created by ivory on 24-7-4.
//

#ifndef CONFIG_H
#define CONFIG_H
#include <cstdlib>
#include <bits/getopt_core.h>
using namespace std;

class Config {
public:
    Config();
    ~Config()= default;
    void parse_arg(int argc, char* argv[]);

    int PORT;// 端口号
    int log_write_way;// 日志写入方式，分为同步和异步
    int trig_mode;// 触发组合模式
    int listen_trig_mode;// LISTENfd触发模式
    int connect_trig_mode;// CONNECTfd触发模式
    int opt_linger;// 优雅关闭链接
    int sql_pools_num;// 数据库连接池数量
    int thread_num;// 线程池内的线程数量
    int is_close_log;// 是否关闭日志
    int actor_model;// 并发模型选择
};

#endif //CONFIG_H
