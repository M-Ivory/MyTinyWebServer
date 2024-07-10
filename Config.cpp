//
// Created by ivory on 24-7-4.
//

#include "Config.h"


Config::Config() {
    PORT = 9006;
    log_write_way = 0;// 日志写入方式，默认同步
    trig_mode = 0;// 触发组合模式,默认listenfd LT + connfd LT
    listen_trig_mode = 0;// listenfd触发模式，默认LT
    connect_trig_mode = 0;// connfd触发模式，默认LT
    opt_linger = 0;// 优雅关闭链接，默认不使用
    sql_pools_num = 8;// 数据库连接池数量,默认8
    thread_num = 8;// 线程池内的线程数量,默认8
    is_close_log = 0;// 是否关闭日志,默认不关闭
    actor_model = 0;// 并发模型,默认是proactor
}

void Config::parse_arg(int argc, char *argv[]) {// 用于解析命令行参数。
    // int argc: 参数个数，代表命令行中参数的数量。
    // char* argv[]: 参数数组，包含了命令行中传递的每个参数。argv[0] 通常是程序的名称，argv[1] 开始是实际的命令行参数。
    int opt;
    const char* str = "p:l:m:o:s:t:c:a:";// 表示可以接受的选项和每个选项是否需要参数。例如，p: 表示 -p 选项需要一个参数。
    while((opt = getopt(argc, argv, str)) != -1) {
        // getopt 每次调用都会返回一个选项字符。如果找到选项，则返回选项字符；如果没有更多选项，则返回 -1
        switch (opt) {
            case 'p':
            {
                PORT = atoi(optarg);// optarg为全局变量，指向当前选项的参数值
                break;
            }
            case 'l':
            {
                log_write_way = atoi(optarg);
                break;
            }
            case 'm':
            {
                trig_mode = atoi(optarg);
                break;
            }
            case 'o':
            {
                opt_linger = atoi(optarg);
                break;
            }
            case 's':
            {
                sql_pools_num = atoi(optarg);
                break;
            }
            case 't':
            {
                thread_num = atoi(optarg);
                break;
            }
            case 'c':
            {
                is_close_log = atoi(optarg);
                break;
            }
            case 'a':
            {
                actor_model = atoi(optarg);
                break;
            }
            default:
                break;
        }
    }
    // 整个函数的作用是解析命令行参数，根据用户输入的选项和参数值来设置配置对象的属性。
    // 例如，如果命令行输入是：./program -p 8080 -l 1 -m 2 -o 1 -s 5 -t 10 -c 0 -a 1则：

    // PORT 会被设置为 8080
    // log_write_way 会被设置为 1
    // trig_mode 会被设置为 2
    // 依此类推。


}
