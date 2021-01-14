#include <iostream>
#include <unistd.h>
#include <error.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <memory.h>
#include <iostream>
#include "tcp_handle.h"
#include "base/net_endian.h"
namespace basic{
namespace {
const int kBufferSize=1500;    
}
class SocketUtil{
public:
static size_t ToSockAddrStorageHelper(sockaddr_storage* addr,
                                      const basic::IpAddress& ip,
                                      uint16_t port,
                                      int scope_id) {
  memset(addr, 0, sizeof(sockaddr_storage));
  addr->ss_family = static_cast<unsigned short>(ip.AddressFamilyToInt());
  if (addr->ss_family == AF_INET6) {
    sockaddr_in6* saddr = reinterpret_cast<sockaddr_in6*>(addr);
    saddr->sin6_addr = ip.GetIPv6();
    saddr->sin6_port =basic::QuicheEndian::HostToNet16(port);
    saddr->sin6_scope_id = scope_id;
    return sizeof(sockaddr_in6);
  } else if (addr->ss_family == AF_INET) {
    sockaddr_in* saddr = reinterpret_cast<sockaddr_in*>(addr);
    saddr->sin_addr = ip.GetIPv4();
    saddr->sin_port = basic::QuicheEndian::HostToNet16(port);
    return sizeof(sockaddr_in);
  }
  return 0;
}
};
MockEndpoint::MockEndpoint(basic::BaseContext *context,int fd,int id):
context_(context),fd_(fd),id_(id){
    OpenTrace();
    context_->epoll_server()->RegisterFD(fd_, this, EPOLLIN|EPOLLET);
}
MockEndpoint::~MockEndpoint(){
    CloseTrace();
    std::cout<<"endpoint destroy "<<id_<<" "<<recv_bytes_<<std::endl;
}
void MockEndpoint::OnRegistration(basic::EpollServer* eps, int fd, int event_mask){}
void MockEndpoint::OnModification(int fd, int event_mask){}
void MockEndpoint::OnEvent(int fd, basic::EpollEvent* event){
    if(event->in_events & EPOLLIN){
        OnReadEvent(fd);
    }    
}
void MockEndpoint::OnUnregistration(int fd, bool replaced){
    Close();
    DeleteSelf();
}
void MockEndpoint::OnShutdown(basic::EpollServer* eps, int fd){
    Close();
    DeleteSelf();
}
void MockEndpoint::OnReadEvent(int fd){
    char buffer[kBufferSize];
    while(true){
        size_t nbytes=read(fd,buffer,kBufferSize);  
        //https://github.com/eklitzke/epollet/blob/master/poll.c  
        if (nbytes == -1) {
            //if(errno == EWOULDBLOCK|| errno == EAGAIN){}
            break;            
        }else if(nbytes==0){
            std::cout<<"close connection"<<std::endl;
            context_->epoll_server()->UnregisterFD(fd_);            
        }else{
             std::cout<<"MockEndpoint::OnReadEvent "<<nbytes<<std::endl;
             recv_bytes_+=nbytes;
        }       
    }    
}
void MockEndpoint::Close(){
    if(fd_>0){
        close(fd_);
        fd_=-1;
    }    
}
void MockEndpoint::OpenTrace(){
    std::string name ="client_"+std::to_string(id_)+"_.txt";
    m_trace.open(name.c_str(), std::fstream::out);      
}
void MockEndpoint::CloseTrace(){
    if(m_trace.is_open()){
        m_trace<<recv_bytes_<<std::endl;
        m_trace.close();
    }
}
void MockEndpoint::DeleteSelf(){
    if(destroyed_){
        return;
    }
    destroyed_=true;

    context_->PostTask([this]{
        delete this;
    });
}
void MockBackend::CreateEndpoint(basic::BaseContext *context,int fd){
    MockEndpoint *endpoint=new MockEndpoint(context,fd,id_++);
    UNUSED(endpoint);
}
PhysicalSocketServer::PhysicalSocketServer(basic::BaseContext *context, std::unique_ptr<Backend> backend)
:context_(context),backend_(std::move(backend)){}
PhysicalSocketServer::~PhysicalSocketServer(){
    context_->epoll_server()->UnregisterFD(fd_);
    Close();
}
bool PhysicalSocketServer::Create(int family, int type){
    fd_=::socket(family, type, 0);
    return fd_>=0;
}
int PhysicalSocketServer::Bind(basic::IpAddress &ip,uint16_t port){
    sockaddr_storage addr_storage;
    size_t len=SocketUtil::ToSockAddrStorageHelper(&addr_storage,ip,port,0);
    sockaddr* addr = reinterpret_cast<sockaddr*>(&addr_storage);
    int err=-1;
    if(fd_>=0){
       err=::bind(fd_, addr, static_cast<int>(len));
    }
    return err;   
}
int PhysicalSocketServer::Listen(int backlog){
    int err=-1;
    if(fd_>=0){
        err = ::listen(fd_, backlog);
        if(err==0){
            context_->epoll_server()->RegisterFD(fd_, this,EPOLLIN);
        }
    }
    return err;
}
int PhysicalSocketServer::SetSocketOption(int level, int optname,const void *optval, socklen_t optlen){
    int ret=-1;
    if(fd_>=0){
        ret=::setsockopt(fd_,level,optname,optval,optlen);
    }
    return ret;
}
int PhysicalSocketServer::GetSocketOption(int level, int optname, void *optval, socklen_t *optlen){
    int ret=-1;
    if(fd_>=0){
        ret=::getsockopt(fd_,level,optname,optval,optlen);
    }
    return ret;
}
void PhysicalSocketServer::OnRegistration(basic::EpollServer* eps, int fd, int event_mask){}
void PhysicalSocketServer::OnModification(int fd, int event_mask){}
void PhysicalSocketServer::OnEvent(int fd, basic::EpollEvent* event){
    if(event->in_events & EPOLLIN){
        OnReadEvent(fd);
    }        
}
void PhysicalSocketServer::OnUnregistration(int fd, bool replaced){
    Close();
}
void PhysicalSocketServer::OnShutdown(basic::EpollServer* eps, int fd){
    Close();
}
void PhysicalSocketServer::OnReadEvent(int fd){
    sockaddr_storage addr_storage;
    socklen_t addr_len = sizeof(addr_storage);
    sockaddr* addr = reinterpret_cast<sockaddr*>(&addr_storage);
    int s=Accept(addr,&addr_len);
    std::cout<<"PhysicalSocketServer::OnReadEvent "<<s<<std::endl;
    if(s>=0){
        backend_->CreateEndpoint(context_,s);
    }
}
int PhysicalSocketServer::Accept(sockaddr* addr,socklen_t* addrlen){
    
     return ::accept(fd_, addr, addrlen);
}
void PhysicalSocketServer::Close(){
    if(fd_>0){
        close(fd_);
        fd_=-1;
    }    
}
}
