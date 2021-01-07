#include "base/cmdline.h"
#include "base/socket_address.h"
#include "base/epoll_api.h"
#include "tcp/tcp_client.h"
#include <unistd.h>
#include <string>
#include <sys/socket.h>
#include <signal.h>
/*
    ./t_asyn -l 127.0.0.1 -r 127.0.0.1  -p 3333
*/
using namespace basic;
using namespace std;
static volatile bool g_running=true;
void signal_exit_handler(int sig)
{
    g_running=false;
}
int main(int argc, char *argv[]){
    signal(SIGTERM, signal_exit_handler);
    signal(SIGINT, signal_exit_handler);
    signal(SIGTSTP, signal_exit_handler); 
    cmdline::parser a;
    a.add<string>("remote", 'r', "remote ip", false, "127.0.0.1");
    a.add<string>("local", 'l', "local ip", false, "0.0.0.0");
    a.add<uint16_t>("port", 'p', "port number", false, 3333, cmdline::range(1, 65535));    
    a.parse_check(argc, argv);
    std::string remote_str=a.get<string>("remote");
    std::string local_str=a.get<string>("local");
    uint16_t port=a.get<uint16_t>("port");
    IpAddress src_ip;
    src_ip.FromString(local_str);
    SocketAddress src_addr(src_ip,0);
    
    IpAddress dst_ip;
    dst_ip.FromString(remote_str);
    SocketAddress dst_addr(dst_ip,port);
    
    basic::EpollServer *epoll_server(new basic::EpollServer());
    basic::TcpClient *client(new basic::TcpClient(epoll_server));
    bool success=client->AsynConnect(src_addr,dst_addr);
    if(success){
        while(g_running){
            epoll_server->WaitForEventsAndExecuteCallbacks();
        }
    }
    delete client;
    delete epoll_server;
    return 0;
}
