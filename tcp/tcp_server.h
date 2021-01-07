#pragma once
#include <utility>
#include <memory>
#include <deque>

#include "tcp/tcp_handle.h"
#include "base/base_context.h"
#include "base/base_epoll_clock.h"
#include "base/base_epoll_alarm_factory.h"
#include "base/base_mutex.h"
namespace basic{
class SocketServerFactory{
public:
    ~SocketServerFactory(){}
    virtual PhysicalSocketServer* CreateSocketServer(BaseContext *context)=0;
};
class TcpServer:public BaseContext{
public:
    TcpServer(std::unique_ptr<SocketServerFactory> factory);
    ~TcpServer();
    bool Init(basic::IpAddress &ip,uint16_t port);
    void HandleEvent();
    PhysicalSocketServer *socket_server();
    //BaseContext
    const basic::QuicClock *clock() override;
    basic::BaseAlarmFactory* alarm_factory() override;
    basic::EpollServer* epoll_server() override;
    void PostInnerTask(std::unique_ptr<basic::QueuedTask> task) override;
private:
    void ExecuteTask();
    std::unique_ptr<SocketServerFactory> socket_server_factory_;
    basic::EpollServer epoll_server_;
    std::unique_ptr<basic::EpollClock> clock_;
    std::unique_ptr<basic::BaseEpollAlarmFactory> alarm_factory_;
    std::unique_ptr<PhysicalSocketServer> socket_server_;
    mutable basic::Mutex task_mutex_;
    std::deque<std::unique_ptr<basic::QueuedTask>>  queued_tasks_;    
};    
}
