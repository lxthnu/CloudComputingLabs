#ifndef MYTHREADPOOL_H
#define MYTHREADPOOL_H

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
#include "taskqueue.h"

// 定义任务结构体
using namespace std;
namespace mythreadpool{
class ThreadPool
{
public:
    ThreadPool( int maxnum){
        // 实例化任务队列
    m_taskQ = new taskqueue::TaskQueue;
    do {
        // 初始化线程池
        
        m_maxNum = maxnum;
        m_busyNum = 0;
        m_aliveNum = 0;

        // 根据线程的最大上限给线程数组分配内存
        m_threadIDs = new pthread_t[maxnum];
        if (m_threadIDs == nullptr)
        {
            cout << "malloc thread_t[] 失败...." << endl;;
            break;
        }
        // 初始化
        memset(m_threadIDs, 0, sizeof(pthread_t) * maxNum);
        // 初始化互斥锁,条件变量
        if (pthread_mutex_init(&m_lock, NULL) != 0 ||
            pthread_cond_init(&m_notEmpty, NULL) != 0)
        {
            cout << "init mutex or condition fail..." << endl;
            break;
        }

        /////////////////// 创建线程 //////////////////
        // 根据最小线程个数, 创建线程
        for (int i = 0; i < minNum; ++i)
        {
            pthread_create(&m_threadIDs[i], NULL, worker, this);
            cout << "创建子线程, ID: " << to_string(m_threadIDs[i]) << endl;
        }
        // 创建管理者线程, 1个
        pthread_create(&m_managerID, NULL, manager, this);
    } while (0);
    }
    ~ThreadPool();

    // 添加任务
    void addTask(taskqueue::Task task){
        if(m_shutdown) return;
        m_taskQ->addTask(task);
        pthread_cond_signal(&m_notEmpty);
        
    }
    // 获取忙线程的个数
    int getBusyNumber();
    // 获取活着的线程个数
    int getAliveNumber();

private:
    // 工作的线程的任务函数
    static void* worker(void* arg){
        ThreadPool* pool = (ThreadPool*)arg;
        while (true)
        {
            pthread_mutex_lock(&pool->m_lock);
            //任务为空且还在运行，阻塞线程
            while (pool->m_taskQ->taskNumber()==0&& !pool->m_shutdown)
            {
                pthread_cond_wait(&pool->m_notEmpty,&pool->m_lock);
                if(pool->m_exitNum>0){//有线程退出
                    pool->m_exitNum--;
                    pthread_mutex_unlock(&pool->m_lock);
                    pool->threadExit();//单个线程退出
                }
            }

            if(pool->m_shutdown){
            pthread_mutex_unlock(&pool->m_lock);
            pool->threadExit();
        }
        taskqueue::Task task=pool->m_taskQ->takeTask();
        pool->m_busyNum++;
        pthread_mutex_unlock(&pool->m_lock);
        printf("thread %ld start working\n",pthread_self());


        task.fun(task.arg);
        delete task.arg;
        task.arg=nullptr;
        
        printf("thread %ld end working\n",pthread_self());

        pthread_mutex_lock(&pool->m_lock);
        pool->m_busyNum--;
        pthread_mutex_unlock(&pool->m_lock); 
            
        }
        return;     
        
    }

    // 管理者线程的任务函数
    static void* manager(void* arg);
    void threadExit(){
        pthread_t tid = pthread_self();
        for(int i=0;i<m_maxNum;i++){
            if(m_threadIDs[i]==tid){
                m_threadIDs[i]=0;
                printf("threadExit() called %ls exit\n",to_string(tid));
            }
        }
        pthread_exit(NULL);
    }

private:
    pthread_mutex_t m_lock;
    pthread_cond_t m_notEmpty;
    pthread_cond_t m_notfull;

    pthread_t* m_threadIDs;
    pthread_t m_managerID;
    taskqueue::TaskQueue* m_taskQ;
    int m_minNum;
    int m_maxNum;
    int m_busyNum;
    int m_aliveNum;
    int m_exitNum;
    bool m_shutdown = false;
};

}
#endif
