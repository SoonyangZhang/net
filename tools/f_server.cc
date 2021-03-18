#include <signal.h>
#include <string.h>
#include <atomic>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "tcp/tcp_server.h"
#include "base/cmdline.h"
#include "base/byte_codec.h"
#include "base/ip_address.h"
#include "base/epoll_api.h"
#include "base/file_op.h"
namespace{
    const int kBufferSize=1500;
    const std::string delimiter="/";
}
namespace basic{
std::string  currentDir(){
    char buf[FILENAME_MAX];
    memset(buf,0,FILENAME_MAX);        
    std::string dir=std::string (getcwd(buf, FILENAME_MAX));
    return dir;
}
class FileBackend: public Backend{
public:
    FileBackend(const std::string &folder);
    void CreateEndpoint(BaseContext *context,int fd) override;
private:
    const std::string &folder_;
};
class FileSocketFactory:public SocketServerFactory{
public:
    FileSocketFactory(const std::string &folder);
    PhysicalSocketServer* CreateSocketServer(BaseContext *context) override;
private:
    const std::string &folder_;
};
class FileEndpoint:public EpollCallbackInterface{
public:
    FileEndpoint(BaseContext *context,int fd,const std::string &folder);
    ~FileEndpoint();
    // From EpollCallbackInterface
    void OnRegistration(basic::EpollServer* eps, int fd, int event_mask) override{}
    void OnModification(int fd, int event_mask) override {}
    void OnEvent(int fd, basic::EpollEvent* event) override;
    void OnUnregistration(int fd, bool replaced) override {}
    void OnShutdown(basic::EpollServer* eps, int fd) override;
    std::string Name() const override {return "FileEndpoint";}
private:
    void OnWriteEvent(int fd);
    void OnReadEvent(int fd);
    void CloseFd();
    void DeleteSelf();
    void ParserMeta(const char *buffer,int sz);
    void Write2File(const char *buffer,int sz);
    void ReplyDone();
    BaseContext *context_;
    int fd_=-1;
    const std::string &folder_;
    FILE *fp_=nullptr;
    std::atomic<bool> destroyed_{false};
    std::string meta_buf_;
    uint32_t file_bytes_=0;
    uint32_t  write_bytes_=0;
    bool meta_parsed_=false;
    int send_bytes_=0;
    int recv_bytes_=0;
};
FileBackend::FileBackend(const std::string &folder):folder_(folder){}
void FileBackend::CreateEndpoint(BaseContext *context,int fd){
    FileEndpoint*endpoint=new FileEndpoint(context,fd,folder_);
    UNUSED(endpoint);
}
FileSocketFactory::FileSocketFactory(const std::string &folder):folder_(folder){}
PhysicalSocketServer* FileSocketFactory::CreateSocketServer(BaseContext *context){
    std::unique_ptr<FileBackend> backend(new FileBackend(folder_));
    return new PhysicalSocketServer(context,std::move(backend));
}
FileEndpoint::FileEndpoint(BaseContext *context,int fd,const std::string &folder)
:context_(context),
fd_(fd),
folder_(folder){
    context_->epoll_server()->RegisterFD(fd_,this,EPOLLET|EPOLLIN|EPOLLRDHUP|EPOLLERR);
}
FileEndpoint::~FileEndpoint(){
    
    std::cout<<Name()<<" dtor"<<send_bytes_<<" "<<recv_bytes_<<std::endl;
}
void FileEndpoint::OnEvent(int fd, basic::EpollEvent* event){
    if(event->in_events & EPOLLIN){
        OnReadEvent(fd);
    }
    if(event->in_events&EPOLLOUT){
        OnWriteEvent(fd);
    }
}
void FileEndpoint::OnShutdown(basic::EpollServer* eps, int fd){
     CloseFd();
}
void FileEndpoint::OnWriteEvent(int fd){
    std::cout<<"FileEndpoint::OnWriteEvent should not be called"<<std::endl;
}
void FileEndpoint::OnReadEvent(int fd){
    char buffer[kBufferSize];
    while(true){
        int n=read(fd,buffer,kBufferSize);
        if (n== -1) {
            //if(errno == EWOULDBLOCK|| errno == EAGAIN){}
            break;
        }else if(n==0){
            CloseFd();
        }else{
            recv_bytes_+=n;
            if(!meta_parsed_){
                ParserMeta(buffer,n);
            }else{
                Write2File(buffer,n);
            }
        }
    }
}
void FileEndpoint::CloseFd(){
    if(fd_>0){
        context_->epoll_server()->UnregisterFD(fd_);
        close(fd_);
        fd_=-1;
    }
    if(fp_){
        fclose(fp_);
        fp_=nullptr;
    }
    DeleteSelf();
}
void FileEndpoint::DeleteSelf(){
    if(destroyed_){
        return;
    }
    destroyed_=true;
    context_->PostTask([this]{
        delete this;
    });
}
void FileEndpoint::ParserMeta(const char *buffer,int sz){
    int old=meta_buf_.size();
    meta_buf_.resize(old+sz);
    memcpy(&meta_buf_[old],buffer,sz);
    basic::DataReader reader(meta_buf_.data(),meta_buf_.size());
    uint16_t name_sz=0;
    bool success=reader.ReadUInt16(&name_sz);
    if(success&&(name_sz+4<=reader.BytesRemaining())){
        int hz=0;
        hz=sizeof(name_sz)+name_sz+4;
        std::string name;
        name.resize(name_sz);
        reader.ReadBytes(&name[0],name.size());
        reader.ReadUInt32(&file_bytes_);
        meta_parsed_=true;
        std::string pathname=folder_+delimiter+name;
        fp_=fopen(pathname.c_str(),"wb");
        if(!fp_){
            std::cout<<"open file failed "<<pathname<<std::endl;
            CloseFd();
            return ;
        }
        int remain=meta_buf_.size()-hz;
        if (remain>0){
            const char *data=meta_buf_.data()+hz;
            Write2File(data,remain);
        }
        std::string null_str;
        null_str.swap(meta_buf_);
    }
    
}
void FileEndpoint::Write2File(const char *buffer,int sz){
    fwrite(buffer,1,sz,fp_);
    write_bytes_+=sz;
    if(write_bytes_==file_bytes_){
        ReplyDone();
    }
}
void FileEndpoint::ReplyDone(){
    uint32_t buffer;
    basic::DataWriter writer((char*)&buffer,sizeof(buffer));
    writer.WriteUInt32(recv_bytes_);
    send(fd_,(void*)&buffer,sizeof(buffer),0);
    CloseFd();
}
}
using namespace basic;
using namespace std;
static volatile bool g_running=true;
void signal_exit_handler(int sig)
{
    g_running=false;
}
/*
./d_echo
*/
int main(int argc, char *argv[]){
    signal(SIGTERM, signal_exit_handler);
    signal(SIGINT, signal_exit_handler);
    signal(SIGTSTP, signal_exit_handler);
    cmdline::parser a;
    a.add<string>("bi", '\0', "bind ip", false, "127.0.0.1");
    a.add<uint16_t>("bp", '\0', "bind port", false,4567, cmdline::range(1, 65535));
    a.parse_check(argc, argv);
    std::string host=a.get<string>("bi");
    uint16_t port=a.get<uint16_t>("bp");
    IpAddress ip;
    ip.FromString(host);
    std::cout<<host<<":"<<port<<std::endl;
    std::string folder;
    {
        std::string name("transfer");
        std::string cur_dir=currentDir();
        folder=cur_dir+delimiter+name;
        if(!makePath(folder)){
            return -1;
        }
    }
    std::cout<<folder<<std::endl;
    std::unique_ptr<FileSocketFactory> socket_facotry(new FileSocketFactory(folder));
    TcpServer server(std::move(socket_facotry));
    bool success=server.Init(ip,port);
    if(success){
        while(g_running){
            server.HandleEvent();
        }
    }
    return 0;
}
