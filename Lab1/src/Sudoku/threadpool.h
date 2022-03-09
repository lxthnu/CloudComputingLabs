#ifndef THREADPOOL_H
#define THREADPOOL_H

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


using namespace std;
namespace mythreadpool
{
/*线程池结构*/
class ThreadPool
{
public:
  
//explicit禁止编译器执行非预期 (往往也不被期望) 的类型转换
  explicit ThreadPool(string name ){
      myname=name;
      running=false;
      maxq=0;
      
      pthread_mutex_init (&_mutex, NULL);
      pthread_cond_init (&notfull, NULL);
      pthread_cond_init (&notempty, NULL);

  }
  ~ThreadPool(){
      if (running){
        stop();
      }
  }

  typedef std::function<void()> Task;

  
  void start(int numThreads);
  void stop();
  void run(Task t);   //运行任务，往线程池中的任务队列添加任务
  bool isFull() {
      return maxq <= myqueue.size();
  }
  

  void setqSize(int num) { maxq = num; }
  std::string name() { return myname; }
  int queueSize();

  bool running;//线程池是否运行
  int shutdown;
  int cur_queue_size;
  

 private:
 

  Task take();//获取任务函数
  static void* runInThread(void *arg);

  pthread_mutex_t _mutex;//条件变量使用锁
  pthread_cond_t notfull;//线程池非满条件变量
  pthread_cond_t notempty;//线程池非空条件变量

  string myname;
  Task threadtask;
  std::vector<pthread_t> mythreads;
  std::deque<Task> myqueue;

  // 任务队列的最大大小
  int maxq;

}; 
}
#endif
