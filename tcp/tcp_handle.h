#pragma once
#include <stdint.h>
#include <atomic>
#include <string>
#include <iostream>
#include <fstream>
#include <memory>
#include "base/base_context.h"
#include "base/epoll_api.h"
#include "base/ip_address.h"
#include "base/base_magic.h"
namespace basic{
class MockEndpoint:public basic::EpollCallbackInterface{
public:
   MockEndpoint(basic::BaseContext *context,int fd,int id);
   ~MockEndpoint();
    // From EpollCallbackInterface
    void OnRegistration(basic::EpollServer* eps, int fd, int event_mask) override;
    void OnModification(int fd, int event_mask) override;
    void OnEvent(int fd, basic::EpollEvent* event) override;
    void OnUnregistration(int fd, bool replaced) override;
    void OnShutdown(basic::EpollServer* eps, int fd) override;
    std::string Name() const override {return "tcp";}
private:
    void OnReadEvent(int fd);
    void Close();
    void DeleteSelf();
    void OpenTrace();
    void CloseTrace();
    basic::BaseContext* context_=nullptr;
    int fd_=-1;
    int id_;
    std::fstream m_trace;
    int recv_bytes_=0;
    std::atomic<bool> destroyed_{false};
};
class Backend{
public:
    virtual ~Backend(){}
    virtual void CreateEndpoint(basic::BaseContext *context,int fd)=0;
};
class MockBackend:public Backend{
public:
    MockBackend(){}
    ~MockBackend() override{}
    void CreateEndpoint(basic::BaseContext *context,int fd) override;
private:
    int id_=0;
};
class PhysicalSocketServer:public basic::EpollCallbackInterface{
public:    
    PhysicalSocketServer(basic::BaseContext *context, std::unique_ptr<Backend> backend);
    ~PhysicalSocketServer();
    bool Create(int family, int type);
    int Bind(basic::IpAddress &ip,uint16_t port);
    int Listen(int backlog);
    int SetSocketOption(int level, int optname,const void *optval, socklen_t optlen);
    int GetSocketOption(int level, int optname, void *optval, socklen_t*optlen);
    // From EpollCallbackInterface
    void OnRegistration(basic::EpollServer* eps, int fd, int event_mask) override;
    void OnModification(int fd, int event_mask) override;
    void OnEvent(int fd, basic::EpollEvent* event) override;
    void OnUnregistration(int fd, bool replaced) override;
    void OnShutdown(basic::EpollServer* eps, int fd) override;  
    std::string Name() const override {return "tcp";}  
private:
    void OnReadEvent(int fd);
    int Accept(sockaddr* addr,socklen_t* addrlen);
    void Close();
    basic::BaseContext* context_=nullptr;
    std::unique_ptr<Backend> backend_;
    int  fd_=-1;
};
}
