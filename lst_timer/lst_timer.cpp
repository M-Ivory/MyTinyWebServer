//
// Created by ivory on 24-7-15.
//
#include "../http/http_connection.h"
#include "lst_timer.h"

sort_timer_list::~sort_timer_list() {
    util_timer *tmp = head;
    while (tmp)
    {
        head = tmp->next;
        delete tmp;
        tmp = head;
    }
}

void sort_timer_list::add_timer(util_timer *timer) {
    if(!timer)return;
    if(!head) {
        head = tail = timer;
        return;
    }
    if(timer->expire < head->expire) {
        timer->next = head;
        head->prev = timer;
        head = timer;
        return;
    }
    // 之前排除了timer的expire小于head的expire的情况
    add_timer(timer, head);
}

void sort_timer_list::adjust_timer_list(util_timer *timer) {// 用于调整list中序列不按expire升序排的
    if(!timer)return;
    util_timer* tmp = timer->next;
    if(!tmp || timer->expire < tmp->expire)return;// 排序正确
    // assert：此时timer->expire >= timer->next->expire
    if(timer == head) {// 把timer先删除再add用于维持序列顺序
        head = head->next;
        head->prev = NULL;
        timer->next = NULL;
        add_timer(timer, head);
    }else {
        timer->prev->next = timer->next;
        timer->next->prev = timer->prev;
        add_timer(timer, timer->next);
    }
    // 此时使用两个参数的add_timer因为已经保证了timer的expire会大于参数二的expire
}

void sort_timer_list::del_timer(util_timer *timer) {
}

void sort_timer_list::tick() {
}

void sort_timer_list::add_timer(util_timer *timer, util_timer *list_head) {// 将timer按expire升序插入，前提为list_head的expire要小于timer的expire，因为调用的是public的add_timer
    util_timer* prev = list_head;
    util_timer* tmp = prev->next;
    while(tmp) {
        if(timer->expire < tmp->expire) {// 将timer按expire升序插入
            prev->next = timer;
            timer->next = tmp;
            tmp->prev = timer;
            timer->prev = prev;
            break;
        }
        prev = tmp;
        tmp = tmp->next;
    }
    if(!tmp) {
        prev->next = timer;
        timer->prev = prev;
        timer->next = NULL;
        tail = timer;
    }
}
