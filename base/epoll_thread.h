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
    EpollThread(){}
    ~EpollThread();
    void Run() override;
};    
}
