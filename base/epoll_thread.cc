#include "epoll_thread.h"
namespace basic{    
EpollThread::~EpollThread(){
    if(running_){
        Stop();
    }
}
void EpollThread::Run(){
    while(running_){
        HandleEvent();
    }
    ExecuteTask();
}
}
