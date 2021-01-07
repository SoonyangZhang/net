#include "epoll_thread.h"
namespace basic{
EpollThread::EpollThread(){
    clock_.reset(new EpollClock(&epoll_server_));
    alarm_factory_.reset(new BaseEpollAlarmFactory(&epoll_server_));
}    
EpollThread::~EpollThread(){
    if(running_){
        Stop();
    }
}
void EpollThread::Run(){
    while(running_){
        ExecuteTask();
        epoll_server_.WaitForEventsAndExecuteCallbacks();
    }
    ExecuteTask();
}
const QuicClock *EpollThread::clock(){
    return clock_.get();
}
BaseAlarmFactory* EpollThread::alarm_factory(){
    return alarm_factory_.get();
}
EpollServer *EpollThread::epoll_server(){
    return &epoll_server_;
}
void EpollThread::PostInnerTask(std::unique_ptr<QueuedTask> task){
    MutexLock lock(&task_mutex_);
    queued_tasks_.push_back(std::move(task));
}
void EpollThread::ExecuteTask(){
    std::deque<std::unique_ptr<QueuedTask>> tasks;
    {
        MutexLock lock(&task_mutex_);
        tasks.swap(queued_tasks_);
    }
    while(!tasks.empty()){
        tasks.front()->Run();
        tasks.pop_front();
    }    
}
}
