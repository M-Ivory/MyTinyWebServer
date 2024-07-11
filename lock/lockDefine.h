//
// Created by ivory on 24-7-6.
//

#ifndef LOCKDEFINE_H
#define LOCKDEFINE_H
#include <exception>
#include <semaphore.h>
#include <pthread.h>


class sem {//信号量类
    // 信号量的数据类型为结构sem_t，它本质上是一个长整型的数。
    // 还记得OS中的PV操作吗，信号量用于表面资源可用个数
public:
    sem() {// 构造函数
        // 函数sem_init()用来初始化一个信号量。它的原型为：int sem_init __P ((sem_t *__sem, int __pshared, unsigned int __value));信号量用sem_init函数创建的，下面是它的说明：
        // int sem_init(sem_t *sem, int pshared, unsigned int value);sem为指向信号量结构的一个指针；
        // pshared不为0时此信号量在进程间共享，否则只能为当前进程的所有线程共享；value给出了信号量的初始值。

        // 这个函数的作用是对由sem指定的信号量进行初始化，设置好它的共享选项，并指定一个整数类型的初始值。
        // pshared参数控制着信号量的类型。如果 pshared的值是0，就表示它是当前进程的局部信号量；否则，其它进程就能够共享这个信号量。（这个参数 受版本影响）
        // pshared传递一个非零将会使函数调用失败，属于无名信号量。
        // 返回值：创建成功返回0，失败返回-1
        if(sem_init(&my_sem, 0, 0) != 0)
            throw std::exception();
    }
    sem(int num) {// 构造函数
        if(sem_init(&my_sem, 0, num) != 0)
            throw std::exception();
    }
    // 两个构造函数pshared都设置为0，表示该信号量为当前进程的所有线程共享的局部信号量
    ~sem() {
        sem_destroy(&my_sem);
    }
    bool wait() {// P操作，这是一个封装sem_wait的函数，调用这个函数会让调用线程阻塞，直到信号量的值大于0，然后将信号量的值减1。
        // 它的作用是从信号量的值减去一个“1”，但它永远会先等待该信号量为一个非零值（大于0）才开始做减法。
        // （如果对一个值为0的信号量调用sem_wait()，这个函数就会等待，直到有其它线程增加了信号量这个值使它不再是0为止，再进行减1操作。）
        // 返回值：操作成功返回0，失败则返回-1且置errno
        // 参数sem：指向信号量结构的一个指针
        return sem_wait(&my_sem) == 0;
    }
    bool post() {// V操作
        // 功能：它的作用来增加信号量的值。给信号量加1。
        // 返回值：操作成功返回0，失败则返回-1且置errno
        // 参数sem：指向信号量结构的一个指针
        return sem_post(&my_sem) == 0;
    }

private:
    sem_t my_sem{};
};

class locker {// 互斥锁类
public:
    locker() {
        // int pthread_mutex_init(pthread_mutex_t *restrict mutex,const pthread_mutexattr_t *restrict attr);
        // 函数是以动态方式创建互斥锁的，参数attr指定了新建互斥锁的属性。如果参数attr为空(NULL)，则使用默认的互斥锁属性，默认属性为快速互斥锁。
        // 互斥锁的属性在创建锁的时候指定，在LinuxThreads实现中仅有一个锁类型属性，不同的锁类型在试图对一个已经被锁定的互斥锁加锁时表现不同。
        // 函数成功完成之后会返回零，其他任何返回值都表示出现了错误。
        if(pthread_mutex_init(&my_mutex, NULL) != 0)
            throw std::exception();
    }
    ~locker() {
        pthread_mutex_destroy(&my_mutex);
    }
    bool lock() {
        return pthread_mutex_lock(&my_mutex) == 0;
    }
    bool unlock() {
        return pthread_mutex_unlock(&my_mutex) == 0;
    }
    pthread_mutex_t *get() {//使用指针的原因是，指针开销小一点
        return &my_mutex;
    }
private:
    // pthread_mutex_t my_mutex;
    pthread_mutex_t my_mutex;
};

class cond {// 条件变量类
public:
    cond() {
        if(pthread_cond_init(&my_cond, NULL) != 0)
            throw std::exception();
    }
    ~cond() {
        pthread_cond_destroy(&my_cond);
    }
    bool wait(pthread_mutex_t *mutex) {
        return pthread_cond_wait(&my_cond, mutex) == 0;
    }
    bool timewait(pthread_mutex_t *mutex, struct timespec t) {// timespec结构体中有两个变量，一个s一个ns
        return pthread_cond_timedwait(&my_cond, mutex, &t) == 0;// timedwait中的t为 系统时间 + 等待时机，即当前时间之后多少s，即now的基础上加多少，若超过则不等待
    }
    bool signal() {// 单播
        return pthread_cond_signal(&my_cond) == 0;
    }
    bool broadcast() {// 广播
        return pthread_cond_broadcast(&my_cond) == 0;
    }
private:
    pthread_cond_t my_cond;
};

#endif //LOCKDEFINE_H
