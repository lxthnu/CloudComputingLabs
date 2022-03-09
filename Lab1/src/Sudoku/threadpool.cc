#include "threadpool.h"

#include  <errno.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

using namespace mythreadpool;

void ThreadPool::start(int numThreads)
{
  //assert(threads_.empty());
  running = true;
  mythreads.reserve(numThreads);//更改vector的容量,为最大线程个数
  for (int i = 0; i < numThreads; ++i)//循环创建线程
  {
   pthread_create (&mythreads[i], NULL,runInThread,NULL);
  }
  
}

/*销毁线程池，等待队列中的任务不会再被执行，但是正在运行的线程会一直
把任务运行完后再退出*/
void ThreadPool::stop()
{
    
    if (!running)return ;/*防止两次调用*/
    
    /*唤醒所有等待线程，线程池要销毁了*/
    pthread_mutex_lock(&_mutex);
    running=false;
    void * thread_result;

    pthread_cond_broadcast (&notempty);
    pthread_cond_broadcast (&notfull);
    pthread_mutex_unlock(&_mutex);

    //阻塞主线程，等待其它线程执行结束
    for (int i = 0; i < mythreads.size(); ++i){
        pthread_join (mythreads[i], &thread_result);
        //输出获取到的线程的返回值
        printf("%s\n", (char*)thread_result);
    }
    /*销毁等待队列*/
    mythreads.clear();
    /*条件变量和互斥量也别忘了销毁*/
    pthread_mutex_destroy(&_mutex);
    pthread_cond_destroy(&notfull);
    pthread_cond_destroy(&notempty);

}


int ThreadPool::queueSize()
{
  pthread_mutex_lock(&_mutex);
  return myqueue.size();
  pthread_mutex_unlock(&_mutex);
}

void ThreadPool::run(Task task)
{
  if (mythreads.empty())//如果线程池当中的线程是空的
  {
    task();//那么就直接执行任务，不进行在线程池中添加任务
  }
  else
  {
    pthread_mutex_lock(&_mutex);
    
    while (isFull()&&running)
    {
      pthread_cond_wait (&notfull, &_mutex);
    }
    if (!running) return;
    myqueue.push_back(std::move(task));//在任务队列中添加任务
    pthread_cond_signal(&notempty);
    pthread_mutex_unlock(&_mutex);

  }
}

ThreadPool::Task ThreadPool::take()
{
  pthread_mutex_lock(&_mutex);
  while (myqueue.empty() && running)//如果任务队列中的任务是空的，且处于运行状态
  {
    pthread_cond_wait (&notempty, &_mutex);//需要等待任务
     

  }
  //否则说明有任务
  Task task;
  if (!myqueue.empty())
  {
    task = myqueue.front();//取出任务,queue，先进先出
    myqueue.pop_front();//删除任务
    if (maxq > 0)
    {
      pthread_cond_signal(&notfull);
    }
  }
  pthread_mutex_unlock(&_mutex);
  return task;
}




void* ThreadPool::runInThread(void *arg)
{
    while (running)//如果线程队列在运行
    {
      Task task(take());//取出一个任务
      if (task)
      {
        task();
      }
    
}