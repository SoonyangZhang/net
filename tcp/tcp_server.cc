#include "tcp_server.h"
namespace basic{
TcpServer::TcpServer(std::unique_ptr<SocketServerFactory> factory):
socket_server_factory_(std::move(factory)){
    clock_.reset(new basic::EpollClock(&epoll_server_));
    alarm_factory_.reset(new basic::BaseEpollAlarmFactory(&epoll_server_));
    socket_server_.reset(socket_server_factory_->CreateSocketServer(this));
    if(!socket_server_->Create(AF_INET,SOCK_STREAM)){
        socket_server_.reset(nullptr);
    }
}
TcpServer::~TcpServer(){
    ExecuteTask();
}
bool TcpServer::Init(basic::IpAddress &ip,uint16_t port){
    bool success=false;
    if(socket_server_){
        if(socket_server_->Bind(ip,port)!=0){
            return success;
        }
        if(socket_server_->Listen(128)!=0){
            return success;
        }
        success=true;        
    }
    return success;
}
void TcpServer::HandleEvent(){
    ExecuteTask();
    epoll_server_.WaitForEventsAndExecuteCallbacks();
}
PhysicalSocketServer *TcpServer::socket_server(){
    PhysicalSocketServer *socket_ptr=nullptr;
    if(socket_server_){
        socket_ptr=socket_server_.get();
    }
    return socket_ptr;
}
const basic::QuicClock *TcpServer::clock(){
    return clock_.get();
}
basic::BaseAlarmFactory* TcpServer::alarm_factory(){
    return alarm_factory_.get();
}
basic::EpollServer* TcpServer::epoll_server(){
    return &epoll_server_;
}
void TcpServer::PostInnerTask(std::unique_ptr<basic::QueuedTask> task){
    basic::MutexLock lock(&task_mutex_);
    queued_tasks_.push_back(std::move(task));    
}
void TcpServer::ExecuteTask(){
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
