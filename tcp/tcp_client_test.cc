#include "base/cmdline.h"
#include "base/socket_address.h"
#include "base/epoll_time.h"
#include <unistd.h>
#include <string>
#include <sys/socket.h>
/*
    ./t_client -l 127.0.0.1 -r 127.0.0.1  -p 3333
*/
using namespace basic;
using namespace std;
int main(int argc, char *argv[]){
    cmdline::parser a;
    a.add<string>("remote", 'r', "remote ip", false, "0.0.0.0");
    a.add<string>("local", 'l', "local ip", false, "0.0.0.0");
    a.add<uint16_t>("port", 'p', "port number", false, 3333, cmdline::range(1, 65535));    
    a.parse_check(argc, argv);
    std::string remote_str=a.get<string>("remote");
    std::string local_str=a.get<string>("local");
    uint16_t port=a.get<uint16_t>("port");
    struct sockaddr_storage local;
    struct sockaddr_storage remote;
    {
        IpAddress ip;
        ip.FromString(local_str);
        SocketAddress socket_addr(ip,0);
        local=socket_addr.generic_address();
    }
    {
        IpAddress ip;
        ip.FromString(remote_str);
        SocketAddress socket_addr(ip,port);
        remote=socket_addr.generic_address();        
    }
    int sockfd = 0;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }
    size_t addr_size = sizeof(struct sockaddr_storage);
    if(bind(sockfd, (struct sockaddr *)&local, addr_size)<0){
        close(sockfd);
        return 1;
    }
    if (connect(sockfd, (struct sockaddr *)&remote,addr_size) < 0)
    {
       std::cout<<"connect failed" <<sockfd<<std::endl;
       close(sockfd);
       return 1;
    }
    std::cout<<"connected" <<std::endl;
    char sendBuff[1500];
    uint64_t sent_bytes=0;
    int64_t last=TimeMillis();
    for(int i=0;i<80000;i++){
        int ret=write(sockfd, sendBuff,1500);
        sent_bytes+=ret;
    }
    close(sockfd);
    int64_t duration=TimeMillis()-last;
    std::cout<<duration<<" "<<sent_bytes<<std::endl;
    return 0;
}
