#ifndef TASKQUEUE_H
#define TASKQUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <assert.h>
#include <deque>
#include <vector>
#include <iostream>
#include <functional>

// 定义任务结构体
namespace taskqueue{
struct Task
{
    void*(*fun)(void*) ;//函数指针
    void* arg;//函数参数
    Task()
    {
        fun = nullptr;
        arg = nullptr;
    }
    Task(void*(*f)(void*) , void* arg)
    {
        fun = f;
        this->arg = arg;
    }
    
};

// 任务队列
class TaskQueue
{
public:
TaskQueue()
{
    pthread_mutex_init(&m_mutex, NULL);
}

~TaskQueue()
{
    pthread_mutex_destroy(&m_mutex);
}

    // 添加任务
    void addTask(Task& task)
{
    pthread_mutex_lock(&m_mutex);
    myqueue.push_back(task);
    pthread_mutex_unlock(&m_mutex);
}

    void addTask(void*(*f)(void*) , void* arg);
    void addTask(void*(*f)(void*), void* arg)
{
    pthread_mutex_lock(&m_mutex);
    Task task;
    task.fun = f;
    task.arg = arg;
    myqueue.push_back(task);
    pthread_mutex_unlock(&m_mutex);
}

    // 取出一个任务
    Task takeTask()
{
    Task t;
    pthread_mutex_lock(&m_mutex);
    if (myqueue.size() > 0)
    {
        t = myqueue.front();
        myqueue.pop_front();
    }
    pthread_mutex_unlock(&m_mutex);
    return t;
}

    // 获取当前队列中任务个数
    inline int taskNumber()
    {
        return myqueue.size();
    }

private:
    pthread_mutex_t m_mutex;    // 互斥锁
    std::deque<Task> myqueue;   // 任务队列
};
}
#endif