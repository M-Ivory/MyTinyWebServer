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
}

void sort_timer_list::adjust_timer(util_timer *timer) {
}

void sort_timer_list::del_timer(util_timer *timer) {
}

void sort_timer_list::tick() {
}

void sort_timer_list::add_timer(util_timer *timer, util_timer *list_head) {// 将timer按expire升序插入，应该有个前提为list_head的expire要小于timer的？
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
