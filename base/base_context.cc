#include "base_context.h"
#include "base_epoll_clock.h"
#include "base_epoll_alarm_factory.h"
namespace basic{
BaseContext::BaseContext(){
    clock_.reset(new basic::EpollClock(&epoll_server_));
    alarm_factory_.reset(new basic::BaseEpollAlarmFactory(&epoll_server_));    
}
BaseContext::~BaseContext(){
    ExecuteTask();
}
void BaseContext::HandleEvent(){
    ExecuteTask();
    epoll_server_.WaitForEventsAndExecuteCallbacks();
}
void BaseContext::PostInnerTask(std::unique_ptr<QueuedTask> task){
    basic::MutexLock lock(&task_mutex_);
    queued_tasks_.push_back(std::move(task));     
}
void BaseContext::ExecuteTask(){
    std::deque<std::unique_ptr<basic::QueuedTask>> tasks;
    {
        basic::MutexLock lock(&task_mutex_);
        tasks.swap(queued_tasks_);
    }
    while(!tasks.empty()){
        tasks.front()->Run();
        tasks.pop_front();
    }     
}
}