/*
Client side implementation of UDP client-server model 
https://www.geeksforgeeks.org/udp-server-client-implementation-c/
*/
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdbool.h>
#include <chrono>
#include <time.h>
#include <math.h>
#include <signal.h>
#include <stdlib.h> //RAND_MAX
#include <iostream>
int make_nonblocking (int fd){
    int flags, ret;
    flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
    return -1;
    }
    // Set the nonblocking flag.
    flags |= O_NONBLOCK;
    ret = fcntl(fd, F_SETFL, flags);
    if (ret == -1) {
    return -1;
    }
    
    return 0;
}
int64_t WallTimeNowInUsec(){
    std::chrono::system_clock::duration d = std::chrono::system_clock::now().time_since_epoch();    
    std::chrono::microseconds mic = std::chrono::duration_cast<std::chrono::microseconds>(d);
    return mic.count(); 
}
int64_t TimeMillis(){
    return WallTimeNowInUsec()/1000;
}
double e_random(double lambda){
    double ret=0.0;
    double u=0.0;
    do{
        u=(double)rand()/(double)RAND_MAX;;
    }while(u<=0||u>1);
    ret=(-1.0/lambda)*log(u);
    return ret;
}
static volatile bool running=true; 
void signal_exit_handler(int sig)
{
    running=false;
}
const int kBufferSize=1500; 
int  rate_table[]={500000,1000000,1500000,2000000,2500000};
const int64_t rate_duration=10000000;// 5s
int main(int argc, char **argv) {
    signal(SIGTERM, signal_exit_handler);
    signal(SIGINT, signal_exit_handler);
    signal(SIGTSTP, signal_exit_handler);
    if (argc != 3) {
        fprintf(stderr, "Usage: %s hostname port\n", argv[0]);
        exit(1);
    }       
    srand((unsigned)time(NULL));      
    uint16_t port= 1234;
    char buffer[kBufferSize];
    char *server_ip=argv[1];
    port = (uint16_t)atoi(argv[2]);
    int sockfd; 
    struct sockaddr_in     servaddr; 
  
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
    
    memset(&servaddr, 0, sizeof(servaddr)); 
      
    // Filling server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(port); 
    servaddr.sin_addr.s_addr = inet_addr(server_ip); 
    int64_t next_send_time=0;
    int offset=0;
    int bps=rate_table[offset];
    int packet_size=1450;
	double interval=0.0;
	double lambda=0.0;
    int all=sizeof(rate_table)/sizeof(rate_table[0]);
    int64_t next_rate_time=0;
    while(running){
        int64_t now=WallTimeNowInUsec();
        if(next_rate_time==0||now>=next_rate_time){
            bps=rate_table[offset];
            interval=((double)packet_size*8*1000)/(bps);
            lambda=1.0/interval;
            offset=(offset+1)%all;
            next_rate_time=now+rate_duration;
        }        
        if(next_send_time==0||now>=next_send_time){
            sendto(sockfd, (const char *)buffer, packet_size, 
                    0,(const struct sockaddr *)&servaddr,sizeof(servaddr));
            int64_t micro_ts=e_random(lambda)*1000; 
            next_send_time=now+micro_ts;
        }
    }
    close(sockfd); 
    return 0; 
} 
