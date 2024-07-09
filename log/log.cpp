//
// Created by ivory on 24-7-7.
//
#include <string>
#include <sys/time.h>
#include "log.h"

#include <cstring>
#include <pthread.h>
#include <stdarg.h>
#include <unistd.h>
#include <bits/mathcalls.h>
using namespace std;

Log::Log() {
    my_log_count = 0;// 当前行数为0
    is_async = false;// 表示为同步
}

Log::~Log() {
    if(my_log_fp != NULL)fclose(my_log_fp);
}

// 异步需要设置阻塞队列的长度，同步不需要设置
// init基于当前日期和给定的日志文件名，并尝试打开该文件进行追加写操作。如果文件名包含路径，则会保留路径部分，并在文件名前添加日期。
// 例：../../log变成../../2024_7_9log
bool Log::init(const char *file_name, int close_log, int log_buf_size, int log_max_lines, int max_queue_size) {
    // 如果设置了max_queue_size，则使用异步
    if(max_queue_size >= 1) {
        is_async = true;
        my_log_queue = new block_queue<string>(max_queue_size);
        pthread_t tid;
        pthread_create(&tid, NULL, flush_log_thread, NULL);//创建一个线程，执行flush_log_thread函数
    }
    my_close_log = close_log;
    my_log_buf_size = log_buf_size;
    my_log_buf = new char[my_log_buf_size];
    memset(my_log_buf, '\0', my_log_buf_size);
    my_single_log_max_lines = log_max_lines;

    time_t t = time(NULL);// 当前时间
    struct tm *sys_time = localtime(&t);// 转换为当地时间
    struct tm my_time = *sys_time;// 存储了本地时间，包括本地时间信息，类似于年、月、日、时、分、秒等。

    const char *p = strrchr(file_name, '/');// 在字符串file_name中查找最后一个出现的'/'字符。如果找到，返回指向该字符的指针；否则，返回NULL。
    // 即判断是否是完整路径还是只是一个文件名
    char log_full_name[256] = {0};// 完整日志文件名

    if(p == NULL) {// p表示纯文件名
        snprintf(log_full_name, 255, "%d_%02d_%02d_%s", my_time.tm_year + 1900, my_time.tm_mon + 1, my_time.tm_mday, file_name);
        // 将年份_月份_日期_完整文件名写入log_full_name中
    }else {// file_name包含路径
        strcpy(log_name, p + 1);// log_name只存储文件名
        strncpy(dir_name, file_name, p - file_name + 1);// 将路径名存储在dir_name中
        snprintf(log_full_name, 255, "%s%d_%02d_%02d_%s", dir_name, my_time.tm_year + 1900, my_time.tm_mon + 1, my_time.tm_mday, log_name);
        // 路径_年份_月份_日期_文件名
    }

    my_today = my_time.tm_mday;// 今天的日期

    my_log_fp = fopen(log_full_name, "a");
    if(my_log_fp == NULL)return false;
    return true;
}

// level表示不同的功能，用switch区分
// format表示日志内容，...表示日志内容可能不止format，因此要用可变参数
// 此函数只是将日志存入文件和给日志写一个head，其中有时间和日志类型
void Log::write_log(int level, const char *format, ...) {// level表示不同的功能，用switch区分
    struct timeval now = {0, 0};
    gettimeofday(&now, NULL);// 此函数获得从Unix纪元以来的s和us
    time_t t = now.tv_sec;// time_t是一个long类型
    struct tm *sys_time = localtime(&t);// 转换为当地时间
    struct tm my_time = *sys_time;// 存储了本地时间，包括本地时间信息，类似于年、月、日、时、分、秒等。
    // 以上部分代码和以下部分代码作用相同，都是获得时间用于表示当前时间
    // 但由于需要us，所以选择上部分

    // time_t t = time(NULL);// 当前时间
    // struct tm *sys_time = localtime(&t);// 转换为当地时间
    // struct tm my_time = *sys_time;// 存储了本地时间，包括本地时间信息，类似于年、月、日、时、分、秒等。
    char s[16] = {0};// 日志类型
    switch (level) {
        case 0:
            strcpy(s, "[debug]:");break;
        case 1:
            strcpy(s, "[info]:");break;
        case 2:
            strcpy(s, "[warn];");break;
        case 3:
            strcpy(s, "[erro];");break;
        default:
            strcpy(s, "[info];");break;
    }
    my_lock.lock();
    my_log_count++;// 日志数加1
    // 接下来开始写日志
    if(my_today != my_time.tm_mday || my_log_count % my_single_log_max_lines == 0) {
        // 检查当前日期是否与上次记录日志的日期不同，或者日志条目数是否已达到单个日志文件最大行数 my_single_log_max_lines。
        // 如果满足任一条件，则需要创建新日志文件。
        char new_log_name[256] = {0};// 新的日志名
        fflush(my_log_fp);
        fclose(my_log_fp);
        char log_time[16] = {0};// 此用于储存日志名中的时间部分
        snprintf(log_time, 16, "%d_%02d_%02d_", my_time.tm_year + 1900, my_time.tm_mon + 1, my_time.tm_mday);

        if(my_today != my_time.tm_mday) {// 进入if的条件是log时间不匹配，新的时间创建新的日志文件
            snprintf(new_log_name, 255, "%s%s%s", dir_name, log_time, log_name);
            my_today = my_time.tm_mday;
            my_log_count = 0;
        }else {// 进入这的条件是log数量超过单个文件最大log数，创建新log文件
            snprintf(new_log_name, 255, "%s%s%s.%lld", dir_name, log_time, log_name, my_log_count / my_single_log_max_lines);
            // 这会显示如../../2024_7_9log.1 ../../2024_7_9log.2的日志文件
        }
        // 上面的结果是做出一个新的日志文件的名字
        my_log_fp = fopen(new_log_name, "a");
    }
    my_lock.unlock();

    va_list valist;
    va_start(valist, format);
    // va_list 是一个用于处理可变参数列表的类型。va_start(valst, format) 宏初始化 valst，使其指向函数参数列表中format之后的第一个可变参数。

    string log_content;// 注意，这个只是一条日志
    my_lock.lock();

    int n = snprintf(my_log_buf, 48, "%d-%02d-%02d %02d:%02d:%02d.%06ld %s ",
                     my_time.tm_year + 1900, my_time.tm_mon + 1, my_time.tm_mday,
                     my_time.tm_hour, my_time.tm_min, my_time.tm_sec, now.tv_usec, s);// 最后的s为日志类型

    int m =  vsnprintf(my_log_buf + n, my_log_buf_size - n - 1, format, valist);
    // 使用valst中的可变参数将格式化的日志消息，即format即之后的可变参数
    // 写入到my_log_buf从位置n开始的地方。
    my_log_buf[m + n] = '\n';
    my_log_buf[m + n + 1] = '\0';
    log_content = my_log_buf;

    my_lock.unlock();

    if(is_async && !my_log_queue->is_full()) {
        // 进入if条件为，异步且日志队列没满
        // 异步时的日志先放入日志队列中，最后一并写入日志文件，并非生成日志即写入
        my_log_queue->push(log_content);
    }else {// 同步，或者日志队列已满，则直接写入文件
        my_lock.lock();
        fputs(log_content.c_str(), my_log_fp);
        my_lock.unlock();
    }

    va_end(valist);// 结束可变参数的处理，并清理相关资源。
}

void Log::flush() {// 强制用fflush刷新写入流缓冲区
    my_lock.lock();
    fflush(my_log_fp);
    my_lock.unlock();
}


