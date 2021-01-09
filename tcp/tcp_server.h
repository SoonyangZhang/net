#pragma once
#include <utility>
#include <memory>
#include "base/base_context.h"
#include "tcp/tcp_handle.h"
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
    PhysicalSocketServer *socket_server();
private:
    std::unique_ptr<SocketServerFactory> socket_server_factory_;
    std::unique_ptr<PhysicalSocketServer> socket_server_;
};    
}
