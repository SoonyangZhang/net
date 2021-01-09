#include "tcp_server.h"
namespace basic{
TcpServer::TcpServer(std::unique_ptr<SocketServerFactory> factory)
{
    socket_server_factory_=std::move(factory);
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
PhysicalSocketServer *TcpServer::socket_server(){
    PhysicalSocketServer *socket_ptr=nullptr;
    if(socket_server_){
        socket_ptr=socket_server_.get();
    }
    return socket_ptr;
}
}
