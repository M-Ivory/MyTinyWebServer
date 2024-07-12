//
// Created by ivory on 24-7-13.
//
#include "threadpool.h"

template<typename T>
threadpool<T>::threadpool(int actor_model, connection_pool *connpool, int thread_num, int max_request):
my_actor_model(actor_model), my_connection_pool(connpool), my_thread_num(thread_num), my_max_request(max_request), my_threads(NULL)
{
    if(thread_num <= 0 || max_request <= 0)throw exception();
    my_threads = new pthread_t[my_thread_num];
    // if(!my_threads)throw exception();
    for(int i=0; i < my_thread_num; ++i) {
        if(pthread_create(my_threads + i, NULL, worker, this) != 0) {// this为传给worker函数的参数
            // 若线程创建失败
            delete[] my_threads;
            throw exception();
        }
        if(pthread_detach(my_threads[i]) != 0) {
            // pthread_detach用于将线程设置为分离状态，这样线程在结束时会自动释放资源
            delete[] my_threads;
            throw exception();
        }
    }
}

template<typename T>
threadpool<T>::~threadpool() {
    delete[] my_threads;
}

template<typename T>
bool threadpool<T>::append(T *request, int state) {
    my_queue_lock.lock();
    if(my_work_queue.size() >= my_max_request) {
        my_queue_lock.unlock();
        return false;
    }
    request->my_state = state;
    // 此处的my_state定义在要用的T，即httpconn中
    // 没错，此处是为了这蝶醋包的饺子，哈哈
    my_work_queue.push_back(request);
    my_queue_sem.post();// V操作，可用资源+1
    // 此处进入临界区后增加资源数量，才调整信号量，例如根据某种条件决定是否允许更多线程访问资源，我认为信号量应放在锁里面
    my_queue_lock.unlock();
    // my_queue_sem.post();// V操作，可用资源+1
    return true;
}

template<typename T>
bool threadpool<T>::append_p(T *request) {
    my_queue_lock.lock();
    if(my_work_queue.size() >= my_max_request) {
        my_queue_lock.unlock();
        return false;
    }
    my_work_queue.push_back(request);
    my_queue_sem.post();// V操作，可用资源+1
    // 此处进入临界区后增加资源数量，才调整信号量，例如根据某种条件决定是否允许更多线程访问资源，我认为信号量应放在锁里面
    my_queue_lock.unlock();
    // my_queue_sem.post();// V操作，可用资源+1
    return true;
}

template<typename T>
void * threadpool<T>::worker(void *arg) {// 此处arg传入的为一个threadpool类
    threadpool* pool = (threadpool*)arg;
    pool->run();
    return pool;
}

template<typename T>
void threadpool<T>::run() {
    while(true) {
        my_queue_sem.wait();
        // 此时由多个线程通过信号量控制访问资源的数量，然后在进入临界区之前获取互斥锁，信号量应放在锁外面
        my_queue_lock.lock();
        if(my_work_queue.empty()) {// 当前无请求
            my_queue_lock.unlock();
            continue;
        }
        T* request = my_work_queue.front();
        my_work_queue.pop_front();
        my_queue_lock.unlock();
        if(!request)continue;
        if(my_actor_model == 1) {// 并发模型选择为

        }else {// 并发模型选择为

        }
    }
}
