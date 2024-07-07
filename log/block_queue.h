//
// Created by ivory on 24-7-7.
//

#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H
#include <sys/time.h>
#include <pthread.h>
#include <iostream>
#include "../lock/lockDefine.h"
using namespace std;

template<class T>class block_queue {
public:
    block_queue(int max_size = 1000) {
        if(max_size <= 0)exit(-1);
        my_max_size = max_size;
        my_array = new T[my_max_size];
        my_size = 0;
        my_front = -1;
        my_back = -1;// 表示为空
    }
    ~block_queue() {
        my_lock.lock();
        if(my_array != NULL) destroy(my_array);
        my_lock.unlock();
    }
    void clear() {// 清空
        my_lock.lock();
        my_size = 0;
        my_front = -1;
        my_back = -1;
        my_lock.unlock();
    }
    bool is_full() {
        my_lock.lock();
        if(my_size >= my_max_size) {
            my_lock.unlock();
            return true;
        }
        my_lock.unlock();
        return false;
    }
    bool is_empty() {
        my_lock.lock();
        if(my_size == 0) {
            my_lock.unlock();
            return true;
        }
        my_lock.unlock();
        return false;
    }
    bool front(T &value) {
        my_lock.lock();
        if(my_size == 0) {
            my_lock.unlock();
            return false;
        }
        value = my_array[my_front];
        my_lock.unlock();
        return true;
    }
    bool back(T &value) {
        my_lock.lock();
        if(my_size == 0) {
            my_lock.unlock();
            return false;
        }
        value = my_array[my_back];
        my_lock.unlock();
        return true;
    }
    // 这里两个get函数要加锁的原因是害怕未读出就被修改了
    int get_size() {
        my_lock.lock();
        int res = my_size;
        my_lock.unlock();
        return res;
    }
    int get_max_size() {
        my_lock.lock();
        int res = my_max_size;
        my_lock.unlock();
        return res;
    }
    // push不管满没满，都要广播一下有东西可以pop
    bool push(const T &value) {
        my_lock.lock();
        if(my_size >= my_max_size) {
            my_cond.broadcast();
            my_lock.unlock();
            return false;
        }
        my_array[my_back] = value;
        my_size++;
        my_back = (my_back + 1) % my_max_size;//循环队列
        my_cond.broadcast();
        my_lock.unlock();
        return true;
    }
    bool pop(T &value) {
        my_lock.lock();
        while(my_size <= 0) {
            // 若wait成功执行，则可以进行pop操作；否则unlock返回false退出
            if(!my_cond.wait(my_lock.get())) {// wait包含一个unlock，而broadcast和signal则包含lock
                // my_cond.wait() 会释放互斥锁并且阻塞当前线程，直到 my_cond 被 boradcast通知，然后重新获取互斥锁并继续执行。
                // 如果 wait() 返回 false（比如在超时或者条件变量被假唤醒的情况下），说明等待失败，需要处理失败的情况。
                my_lock.unlock();// 通常情况下，wait 方法会自动管理互斥锁的释放和重新获取，但在失败的情况下，我们需要手动释放。
                return false;
            }
        }
        my_front = (my_front + 1) % my_max_size;
        value = my_array[my_front];
        my_size--;
        my_lock.unlock();
        return true;
    }
    bool pop(T &value, int ms_timeout) {// ms_timeout为最大运行ms
        struct timespec t = {0, 0};// 规定超时时间
        struct timeval now = {0, 0};// 当前系统时间
        gettimeofday(&now, NULL);// 此函数指用于timeval
        my_lock.lock();
        if(my_size <= 0) {
            t.tv_sec = now.tv_sec + ms_timeout / 1000;// 当前系统时间 + 预计超时时间ms中换算成s的部分
            t.tv_nsec = (ms_timeout % 1000) * 1e6;// 取预计超时时间ms中换算成ns的部分
            if(!my_cond.timewait(my_lock.get(), t)) {
                my_lock.unlock();
                return false;
            }
        }
        if(my_size <= 0) {// timewait完若还是空的，则还是false退出
            my_lock.unlock();
            return false;
        }
        my_front = (my_front + 1) % my_max_size;
        value = my_array[my_front];
        my_size--;
        my_lock.unlock();
        return true;
    }

private:
    locker my_lock;
    cond my_cond;

    T* my_array;// 存储队列
    int my_size;
    int my_max_size;
    // my_front指队首前一个元素
    // my_back指队尾元素
    int my_front;
    int my_back;
};
#endif //BLOCK_QUEUE_H
