#include "background.hpp"
#include "autolocker.hpp"
#include <TSPR/concurrent_queue.h>

#include <mutex>
#include <thread>

#ifdef _WIN32

#include <Windows.h>
#define MILLISLEEP(X) SleepEx(X, TRUE)

#else

#include <unistd.h>
#define MILLISLEEP(X) usleep(1000 * X)

#endif

namespace Kashyyyk{

Task::Task(){repeating = false;}
Task::~Task(){}

//! @cond

class Thread::TaskGroup{
  public:
    concurrent_queue<Task *> queue;
};

struct Thread::Thread_Impl{
    concurrent_queue<Task *> *queue;
    std::mutex mutex;
    std::thread *thread;
    bool live;
};

//! @endcond

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


Thread::TaskGroup *Thread::GetShortThreadPool(){
    static TaskGroup group;
    return &group;
}


Thread::TaskGroup *Thread::GetLongThreadPool(){
    static TaskGroup group;
    return &group;
}


void Thread::AddLongRunningTask(Task *task){
    Thread::GetLongThreadPool()->queue.push(task);
}


void Thread::AddShortRunningTask(Task *task){
    Thread::GetShortThreadPool()->queue.push(task);
}


Thread::Thread(TaskGroup *group)
  : guts(new Thread::Thread_Impl()){
    guts->queue = &(group->queue);
    guts->live = true;
    guts->thread = new std::thread((void(*)(Thread::Thread_Impl *))ThreadFunction, guts.get());

}


Thread::~Thread(){
    guts->live = false;
    guts->thread->join();
    delete guts->thread;
}

void Thread::AddTask(TaskGroup *pool, Task *task){
    pool->queue.push(task);
}

void Thread::PerformTask(TaskGroup *pool){
    while(true){
        Task * task;
        if(!pool->queue.try_pop(task))
          break;

        task->Run();

        if(task->repeating)
          pool->queue.push(task);
        else
          delete task;
    }
}

Thread::TaskGroup *Thread::CreateTaskGroup(){
    return new Thread::TaskGroup();
}
void Thread::DestroyTaskGroup(Thread::TaskGroup *a){
    delete a;
}

}
