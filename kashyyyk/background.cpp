#include "background.hpp"
#include "autolocker.hpp"
#include <TSPR/concurrent_queue.h>

#include <mutex>
#include <thread>

#ifdef _WIN32

#include <Windows.h>
#define MILLISLEEP(X) Sleep(X)

#else

#include <unistd.h>
#define MILLISLEEP(X) usleep(1000 * X)

#endif

namespace Kashyyyk{

Task::Task(){repeating = false;}
Task::~Task(){}

void Task::Run(){
    MILLISLEEP(10);
}


class ThreadPool{
  public:
    concurrent_queue<Task *> queue;
};


struct Thread::Thread_Impl{
    concurrent_queue<Task *> *queue;
    std::mutex mutex;
    std::thread *thread;
    bool live;
};


static void ThreadFunction(Thread::Thread_Impl *thimble){
    while(true){
        Task * task;

        MILLISLEEP(10);

        {
            AutoLocker<std::mutex *> locker(&thimble->mutex);
            if(!thimble->live)
              break;
        }

        if(!thimble->queue->try_pop(task))
          continue;

        task->Run();

        if(task->repeating)
          thimble->queue->push(task);
        else
          delete task;

    }
}


ThreadPool *Thread::GetShortThreadPool(){
    static ThreadPool pool;
    return &pool;
}


ThreadPool *Thread::GetLongThreadPool(){
    static ThreadPool pool;
    return &pool;
}


void AddLongRunningTask(Task *task){
    Thread::GetLongThreadPool()->queue.push(task);
}


void AddShortRunningTask(Task *task){
    Thread::GetShortThreadPool()->queue.push(task);
}


Thread::Thread(ThreadPool *pool)
  : guts(new Thread::Thread_Impl()){
    guts->queue = &(pool->queue);
    guts->live = true;
    guts->thread = new std::thread((void(*)(Thread::Thread_Impl *))ThreadFunction, guts.get());

}


Thread::~Thread(){
    guts->live = false;
    guts->thread->join();
    delete guts->thread;
}

}
