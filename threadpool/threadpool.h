//
// Created by ivory on 24-7-13.
//

#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include <sys/time.h>
#include "../lock/lockDefine.h"
#include "../mysql_pool/sql_connection_pool.h"
using namespace std;

template <typename T> class threadpool {
public:
    threadpool(int actor_model, connection_pool* connpool, int thread_num = 8, int max_request = 10000);
    // actor_model是并发模型选择，connpool连接池
    // thread_number是线程池中线程的数量
    // max_requests是请求队列中最多允许的、等待处理的请求的数量
    ~threadpool();
    bool append(T* request, int state);
    bool append_p(T* request);

private:
    static void* worker(void* arg);// 工作线程运行的函数，它不断从工作队列中取出任务并执行
    void run();

private:
    int my_thread_num;// 线程数
    int my_max_request;// 最大请求数
    pthread_t* my_threads;// pthread_t类型的数组，描述线程池，大小为my_thread_num
    list<T *>my_work_queue;// 请求队列，用于httpconn中，
    locker my_queue_lock;
    sem my_queue_sem;
    connection_pool* my_connection_pool;//数据库连接池
    int my_actor_model;// 并发模型

};

#endif //THREADPOOL_H
