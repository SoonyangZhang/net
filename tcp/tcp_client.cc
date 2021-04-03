#include <unistd.h>
#include <error.h>
#include <sys/types.h>
#include "tcp_client.h"
#include <iostream>
namespace basic{
TcpClient::TcpClient(basic::EpollServer *eps):eps_(eps){}
TcpClient::~TcpClient(){
    status_=TCP_DISCONNECT;
    eps_->UnregisterFD(fd_);
    Close();
}
bool TcpClient::AsynConnect(basic::SocketAddress &local,basic::SocketAddress& remote){
    src_addr_=local.generic_address();
    dst_addr_=remote.generic_address();
    int yes=1;
    bool success=false;
    if ((fd_ = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        return success;
    }
    if(setsockopt(fd_,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int))!=0){
        Close();
        return success;        
    }
    size_t addr_size = sizeof(struct sockaddr_storage);
    if(bind(fd_, (struct sockaddr *)&src_addr_, addr_size)<0){
        Close();
        return success;
    }
    eps_->RegisterFD(fd_, this,EPOLLOUT | EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLET);
    if(connect(fd_,(struct sockaddr *)&dst_addr_,addr_size) == -1&& errno != EINPROGRESS){
        //connect doesn't work, are we running out of available ports ? if yes, destruct the socket   
        if (errno == EAGAIN){
            eps_->UnregisterFD(fd_);
            Close();
            return success;                
        }   
    }
    status_=TCP_CONNECTING;
    return true;
}
void TcpClient::OnEvent(int fd, basic::EpollEvent* event){
    if (event->in_events&(EPOLLERR|EPOLLRDHUP | EPOLLHUP)){
        int status, err;
        socklen_t len=sizeof(err);
        status = ::getsockopt(fd_, SOL_SOCKET, SO_ERROR, &err, &len);
        eps_->UnregisterFD(fd_);
        Close();
        std::cout << status << " " << err <<" "<<count_<<std::endl;        
    }   
    if(event->in_events&EPOLLOUT){
        if(status_==TCP_CONNECTING){
            status_=TCP_CONNECTED;
            eps_->ModifyCallback(fd_,EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLET);
            OnCanWrite();
        }
    }
    count_++;
}
void TcpClient::OnShutdown(basic::EpollServer* eps, int fd){
    Close();
}
void TcpClient::OnCanWrite(){
    char sendBuff[1500];
    for(int i=0;i<12;i++){
        int ret=write(fd_, sendBuff,1500); 
    }
    eps_->UnregisterFD(fd_);
    Close();
}
void TcpClient::Close(){
    if(fd_>0){
        status_=TCP_DISCONNECT;
        close(fd_);
        fd_=-1;        
    }      
}
}
