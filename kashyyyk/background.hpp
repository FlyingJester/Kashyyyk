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

class TaskGroup;

void AddTask(TaskGroup *, Task *);
void PerformTask(TaskGroup *);

void AddLongRunningTask(Task *);
void AddShortRunningTask(Task *);

TaskGroup *CreateTaskGroup();
void DestroyTaskGroup(TaskGroup *);

class Thread{
public:

  Thread(TaskGroup *);
  ~Thread();


  static TaskGroup *GetShortThreadPool();
  static TaskGroup *GetLongThreadPool();

  struct Thread_Impl;
  std::unique_ptr<Thread_Impl> guts;

};

}
