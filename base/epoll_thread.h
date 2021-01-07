#pragma once
#include <utility>
#include <memory>
#include <deque>

#include "base_thread.h"
#include "base_context.h"
#include "base_epoll_clock.h"
#include "base_epoll_alarm_factory.h"
#include "base_mutex.h"
namespace basic{
class EpollThread :public BaseThread,
public BaseContext{
public:
    EpollThread();
    ~EpollThread();
    void Run() override;
    //BaseContext
    const QuicClock *clock() override;
    BaseAlarmFactory* alarm_factory() override;
    EpollServer* epoll_server() override;
    void PostInnerTask(std::unique_ptr<QueuedTask> task) override;              
private:
    void ExecuteTask();
    EpollServer epoll_server_;
    std::unique_ptr<EpollClock> clock_;
    std::unique_ptr<BaseEpollAlarmFactory> alarm_factory_;
    mutable Mutex task_mutex_;
    std::deque<std::unique_ptr<QueuedTask>>  queued_tasks_;
};    
}
