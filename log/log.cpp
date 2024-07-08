//
// Created by ivory on 24-7-7.
//
#include <string>
#include <sys/time.h>
#include "log.h"

#include <cstring>
#include <pthread.h>
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
    my_log_max_lines = log_max_lines;

    time_t t = time(NULL);// 当前时间
    struct tm *sys_time = localtime(&t);// 转换为当地时间
    struct tm my_time = *sys_time;// 存储了本地时间，包括本地时间信息，类似于年、月、日、时、分、秒等。

    const char *p = strrchr(file_name, '/');// 在字符串file_name中查找最后一个出现的'/'字符。如果找到，返回指向该字符的指针；否则，返回NULL。
    // 即判断是否是完整路径还是只是一个文件名
    char log_full_name[256] = {0};// 完整日志文件名

    if(p == NULL) {
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

