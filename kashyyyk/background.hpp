#pragma once
#include <memory>

//! @file
//! @brief Definition of @link Kashyyyk::Task @endlink and
//! @link Kashyyyk::Thread @endlink
//! @author    FlyingJester
//! @date      2014
//! @copyright GNU Public License 2.0

namespace Kashyyyk {

//!
//! @brief Basic Task object. Designed to work with Thread and
//! TaskGroup
//!
//! Adding a Task to a TaskGroup using AddTask will result
//! in Run being called by Kashyyyk::PerformTask called with the same
//! TaskGroup.
//!
//! Two default TaskGroup objects exist. They are the Long Running
//! TaskGroup accessed with Thread::GetLongTaskGroup and the Short
//! Running TaskGroup accessed with Thread::GetShortRunningTaskGroup.
//! These are purely organizational, to provide a simple predefined interface
//! withing the Thread class, and are no different in practical use
//! than any other TaskGroup.
//!
//! When the Task is run in the a TaskGroup::TaskGroup, if repeating
//! is true when Run completes the Task will be readded to the TaskGroup
//! automatically. This is useful tof repeating tasks.
//!
//! If repeating is false, the Task will be deleted after it is next performed.
//!
//! It is important that Tasks are relatively short. Longer tasks should be
//! broken up as much as possible. This is important because otherwise ~Thread
//! may become effectively blocking.
class Task {
public:
    Task();
    virtual ~Task();

    //! @brief Override this method to what the Task should do.
    //!
    //! If you want to change the repeating status of the Task, it is best to
    //! do it here and not in the constructor, even for task that never repeat.
    virtual void Run() = 0;

    //! @brief determines if the Task will be reappear to the task queue when
    //! it completes.
    bool repeating;

};

//!
//! @brief Represents a thread
//!
//! Be sure to be careful about stack allocation of Thread objects. Every time
//! one is constructed a new thread is created, and every time one is destroyed
//! the thread is joined again.
//!
//! Threads are signalled to join when they are destroyed.
//!
class Thread{
public:

    //!
    //! @class TaskGroup
    //!
    //! @brief Opaque object representing both a thread pool's identity and an
    //! associated queue of Kashyyyk::Task objects.
    //!
    //! A TaskGroup is created using CreateTaskGroup.
    //!
    class TaskGroup;

    //!
    //! @brief Create a thread
    //!
    //! This task will perform tasks placed on @p taskgroup using #AddTask, or
    //! if @p taskgroup is from #GetShortThreadPool or #GetLongThreadPool,
    //! #AddLongRunningTask and #AddShortRunningTask respectively.
    //! @param taskgroup The TaskGroup to use.
    Thread(TaskGroup *taskgroup);

    //!
    //! @brief Joins the thread
    ~Thread();

    //! @brief Retrieve the predefined Short Running TaskGroup
    //!
    //! Tasks can be added to this task group using AddShortRunningTask.
    //! @return The predefined Short Running Task Group. It is not recommended
    //! to delete this.
    static TaskGroup *GetShortThreadPool();

    //! @brief Retrieve the predefined Long Running TaskGroup
    //!
    //! Tasks can be added to this task group using AddLongRunningTask.
    //! @return The predefined Long Running Task Group. It is not recommended
    //! to delete this.
    static TaskGroup *GetLongThreadPool();

    //!
    //! @brief Add a task to a group
    //!
    //! Pushes @p task into the Task queue in @p group. Some future call
    //! of PerformTask with @p group will perform task.
    //!
    //! @param task to be performed.
    //! @param group to perform the task.
    static void AddTask(TaskGroup *group, Task *task);

    //!
    //! @brief Perform the next task in @p group
    //!
    //! Calling this performs the next task in @p group.
    //! @param group to perform a task from.
    static void PerformTask(TaskGroup *group);

    //! @brief Queue a @p task to be performed in the Long Running TaskGroup
    //! @param Task to be performed
    static void AddLongRunningTask(Task *task);
    //! @brief Queue a @p task to be performed in the Short Running TaskGroup
    //! @param Task to be performed
    static void AddShortRunningTask(Task *task);

    //! @brief Create a new TaskGroup
    //!
    //! This is a very lightweight call, since all thread creation and
    //! initialization is performed when Thread objects are created.s
    //! @return New TaskGroup, ready to be used
    static TaskGroup *CreateTaskGroup();
    //! @brief Destroy a TaskGroup
    //!
    //! @warning All remaining queued tasks are dropped when this is called,
    //! so it is best to ensure that all tasks have completed before this.
    static void DestroyTaskGroup(TaskGroup *task);

    //! @cond

    struct Thread_Impl;

private:

    std::unique_ptr<Thread_Impl> guts;

    //! @endcond

};

}
