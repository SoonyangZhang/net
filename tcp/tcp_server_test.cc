#include <signal.h>
#include <string.h>
#include "tcp/tcp_server.h"
#include "base/cmdline.h"
#include "base/ip_address.h"
/*
    ./t_server -h 127.0.0.1 -p 3333
*/
using namespace basic;
using namespace std;
static volatile bool g_running=true;
void signal_exit_handler(int sig)
{
    g_running=false;
}
namespace basic{
class MockSocketFactory:public SocketServerFactory{
public:
    MockSocketFactory(){}
    PhysicalSocketServer* CreateSocketServer(BaseContext *context){
        std::unique_ptr<MockBackend> backend_(new MockBackend);
        return new PhysicalSocketServer(context,std::move(backend_));
    }
private:
};
}
int main(int argc, char *argv[]){
    signal(SIGTERM, signal_exit_handler);
    signal(SIGINT, signal_exit_handler);
    signal(SIGTSTP, signal_exit_handler);    
    cmdline::parser a;
    a.add<string>("host", 'h', "host name", false, "0.0.0.0");
    a.add<uint16_t>("port", 'p', "port number", false, 3333, cmdline::range(1, 65535));
    a.parse_check(argc, argv); 
    std::string host=a.get<string>("host");
    uint16_t port=a.get<uint16_t>("port");
    IpAddress ip;
    ip.FromString(host);
    std::cout<<host<<" "<<port<<std::endl;
    std::unique_ptr<basic::MockSocketFactory> socket_facotry(new MockSocketFactory);
    TcpServer server(std::move(socket_facotry));
    bool success=server.Init(ip,port);
    if(success){
        while(g_running){
            server.HandleEvent();
        }
    }
    return 0;
}
