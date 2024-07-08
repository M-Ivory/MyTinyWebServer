//
// Created by ivory on 24-7-7.
//

#ifndef LOG_H
#define LOG_H
#include <iostream>
#include <string>
#include <pthread.h>
#include "block_queue.h"

using namespace std;
class Log {
public:
    static Log* get_instance() {// 此函数用于获取一个Log实例，因为Log的构造函数为private
        static Log instance;
        return &instance;
    }
    static void* flush_log_thread(void* args) {// 调用private写日志函数，执行异步日志写操作
        Log::get_instance()->async_write_log();
    }
    bool init(const char* file_name, int close_log, int log_buf_size = 8192, int log_max_lines = 5000000, int max_queue_size = 0);
    void write_log(int level, const char* format, ...);
    void flush(void);
private:
    // 构造函数和析构函数私有化可以
    // 构造函数私有化：防止在类的外部通过 new 或者其他方式创建实例。
    // 析构函数私有化：防止在类的外部通过 delete 直接销毁实例。
    Log();
    virtual ~Log();
    // virtual的作用是，倘若有个子类继承Log，delete删除子类对象时，
    // 若无virtual，则会只调用父类中的析构函数～Log，而不会调用子类的析构函数
    // 若有virtual，则会先调用子类的析构函数，后调用父类中的析构函数～Log
    void* async_write_log() {// 此函数将队列中一条日志写入文件
        string single_log;
        while(my_log_queue->pop(single_log)) {
            my_lock.lock();
            fputs(single_log.c_str(), my_log_fp);
            my_lock.unlock();
        }
    }
private:
    char dir_name[128];// 路径名
    char log_name[128];// 日志名
    int my_log_max_lines;// 日志最大行数
    int my_log_buf_size;// 日志缓冲区大小
    long long my_log_count;// 记录日志行数
    int my_today;// 记录当天
    FILE *my_log_fp;// 打开log的文件指针
    char *my_log_buf;// 日志缓冲区
    block_queue<string> *my_log_queue;// 日志队列
    // 此处是类的指针，而队列在类中；是用指针应该是为了减少开销？
    bool is_async;// 是否异步标志
    locker my_lock;// 互斥锁
    int my_close_log;// 关闭日志
};

// 宏定义，可以直接调用Log
// ##__VA_ARGS__ 是一个预处理器特性，用于处理可变参数宏。在宏定义中，__VA_ARGS__ 代表传递给宏的可变参数部分。
// 在C和C++中，宏可以定义成接受可变数量的参数，这些参数用...表示。在宏定义中，__VA_ARGS__代表所有传递的可变参数。
// 而##__VA_ARGS__中##的作用
// 在某些情况下，如果没有传递任何可变参数，那么在宏展开时会出现一个多余的逗号。为了避免这种情况，使用##操作符可以在__VA_ARGS__为空时删除前面的逗号。

#define LOG_DEBUG(format, ...) if(0 == Log::get_instance()->m_close_log) {Log::get_instance()->write_log(0, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_INFO(format, ...) if(0 == Log::get_instance()->m_close_log) {Log::get_instance()->write_log(1, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_WARN(format, ...) if(0 == Log::get_instance()->m_close_log) {Log::get_instance()->write_log(2, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_ERROR(format, ...) if(0 == Log::get_instance()->m_close_log) {Log::get_instance()->write_log(3, format, ##__VA_ARGS__); Log::get_instance()->flush();}

#endif //LOG_H
