#include "base_context.h"
#include "base_epoll_clock.h"
#include "base_epoll_alarm_factory.h"
namespace basic{
BaseContext::BaseContext(){
    clock_.reset(new basic::EpollClock(&epoll_server_));
    alarm_factory_.reset(new basic::BaseEpollAlarmFactory(&epoll_server_));    
}
BaseContext::~BaseContext(){
	while(!exit_visitors_.empty()){
		auto it=exit_visitors_.begin();
		ExitVisitor* visitor=(*it);
		visitor->ExitGracefully();
		exit_visitors_.erase(it);
	}
    std::set<ExitVisitor*> null_set;
    null_set.swap(exit_visitors_);
    ExecuteTask();
}
void BaseContext::HandleEvent(){
    ExecuteTask();
    epoll_server_.WaitForEventsAndExecuteCallbacks();
}
bool  BaseContext::RegisterExitVisitor(ExitVisitor *visitor){
    bool success=false;
    auto it=exit_visitors_.find(visitor);
    if(it==exit_visitors_.end()){
        exit_visitors_.insert(visitor);
        success=true;
    }
    return success;
}
bool  BaseContext::UnRegisterExitVisitor(ExitVisitor *visitor){
    bool success=false;
    auto it=exit_visitors_.find(visitor);
    if(it!=exit_visitors_.end()){
        exit_visitors_.erase(it);
        success=true;
    }
    return success;
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
