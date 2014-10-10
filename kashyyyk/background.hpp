#pragma once
#include <memory>

namespace Kashyyyk {


class Task {
public:
  Task();
  virtual ~Task();
  virtual void Run();

  bool repeating;

};

void AddLongRunningTask(Task *);
void AddShortRunningTask(Task *);

class ThreadPool;

class Thread{
public:

  Thread(ThreadPool *);
  ~Thread();


  static ThreadPool *GetShortThreadPool();
  static ThreadPool *GetLongThreadPool();

  struct Thread_Impl;
  std::unique_ptr<Thread_Impl> guts;

};

}
