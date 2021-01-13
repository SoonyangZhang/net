#pragma once
#include "base/epoll_api.h"
#include "base/socket_address.h"
#include "tcp/tcp_types.h"
namespace basic{
class TcpClient :public basic::EpollCallbackInterface{
public:
    TcpClient(basic::EpollServer *eps);
    ~TcpClient();
    bool AsynConnect(basic::SocketAddress &local,basic::SocketAddress &remote);
    // From EpollCallbackInterface
    void OnRegistration(basic::EpollServer* eps, int fd, int event_mask) override{}
    void OnModification(int fd, int event_mask) override{}
    void OnEvent(int fd, basic::EpollEvent* event) override;
    void OnUnregistration(int fd, bool replaced) override{}
    void OnShutdown(basic::EpollServer* eps, int fd) override;
    std::string Name() const override {return "tcp";}
private:
    void OnCanWrite();
    void Close();
    basic::EpollServer *eps_=nullptr;
    struct sockaddr_storage src_addr_;
    struct sockaddr_storage dst_addr_;
    int fd_=-1;
    TcpConnectionStatus status_=TCP_STATUS_MIN;
    int count_=0;
};
}