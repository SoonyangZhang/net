#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <cstring>
#include <errno.h>   // for errno and strerror_r
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> //for sockaddr_in
#include <arpa/inet.h>  //inet_addr
#include <netinet/tcp.h> //TCP_NODELAY
#include <sys/stat.h>
#include <fcntl.h>

#include "base/base_ini.h"
#include "base/file_op.h"
#include "base/byte_codec.h"
#include "logging/logging.h"
namespace{
    const int kBufferSize=1500; 
}
using namespace std;
using namespace basic;
int send_file(std::string &path,std::string&name,std::string &ip,uint16_t port){
    int ret=-1;
    int filefd=-1,sockfd=-1;
    std::string pathname=path+name;
    char buffer[kBufferSize]={0};
    int file_bytes=get_file_size(pathname);
    if(file_bytes<=0){
        return ret;
    }
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip.c_str());
    servaddr.sin_port = htons(port);
    int flag = 1;
    if ((sockfd= socket(AF_INET, SOCK_STREAM, 0)) < 0){
        std::cout<<"Error : Could not create socket"<<std::endl;
        return ret;
    }
    setsockopt (sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag));
    if (connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) != 0) {
        std::cout<<"connection with the server failed"<<std::endl;
        close(sockfd);
        sockfd=-1;
        return ret;
    }
    //seems will not happend
    filefd=open(pathname.c_str(),O_RDONLY);
    if(filefd<0){
        close(sockfd);
        sockfd=-1;
        return ret;
    }
    int meta_sz=0;
    {
        basic::DataWriter writer(buffer,kBufferSize);
        bool success=writer.WriteUInt16(name.size())&&
                    writer.WriteBytes(name.data(),name.size())&&
                    writer.WriteUInt32(file_bytes);
        int target=writer.length();
        meta_sz=target;
        int wz=write(sockfd,buffer,target);
        if(wz!=target){
            close(sockfd);
            close(filefd);
            sockfd=-1;
            filefd=-1;
            return ret;
        }
    }
    int send_bytes=0;
    while(true){
        int rz=read(filefd,buffer,kBufferSize);
        if(rz<=0){
            break;
        }
        int off=0;
        while(off<rz){
            int target=rz-off;
            int wz=write(sockfd,buffer+off,target);
            CHECK(wz>=0);
            off+=wz;
            send_bytes+=wz;
        }
    }
    int rz=read(sockfd,buffer,kBufferSize);
    uint32_t recv_bytes=0;
    {
        basic::DataReader reader(buffer,rz);
        reader.ReadUInt32(&recv_bytes);
    }
    
    close(filefd);
    close(sockfd);
    if (send_bytes==file_bytes&&recv_bytes==(file_bytes+meta_sz)){
        ret=0;
    }
    return ret;
}
static volatile bool g_running=true;
void signal_exit_handler(int sig)
{
    g_running=false;
}
/*
 * f_sender  sender.ini
 */
int main(int argc, char *argv[]){
    signal(SIGTERM, signal_exit_handler);
    signal(SIGINT, signal_exit_handler);
    signal(SIGHUP, signal_exit_handler);//ctrl+c
    signal(SIGTSTP, signal_exit_handler);//ctrl+z
    if(argc!=2){
    	return -1;
    }
    const char* sender_ini=argv[1];
    ini_t *config=ini_load(sender_ini);
    const char *files_num=ini_get(config,"files","num");
    std::vector<std::pair<std::string,std::string>> file_names;
    if(files_num){
        int n=std::stoi(files_num);
        for(int i=0;i<n;i++){
            std::string segment=std::string("file")+std::to_string(i+1);
            const char *path=ini_get(config,segment.c_str(),"path");
            if(path){
                const char *name=ini_get(config,segment.c_str(),"name");
                if(name){
                    file_names.push_back(std::make_pair(path,name));
                }
            }
        }
    }
    std::string remote_ip;
    uint16_t remote_port=0;
    {
        const char *ip=ini_get(config,"server","ip");
        if(ip){
            remote_ip=std::string(ip);
            const char *port_str=ini_get(config,"server","port");
            if(port_str){
                remote_port=atoi(port_str);
            }
        }
    }
    ini_free(config);
    for(int i=0;i<file_names.size();i++){
        if(send_file(file_names[i].first,file_names[i].second,remote_ip,remote_port)!=0){
            std::cout<<"send failed: "<<file_names[i].second<<std::endl;
        }
    }
    return 0;
}
