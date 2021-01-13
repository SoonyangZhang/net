#pragma once
#include <stdint.h>
namespace basic{
enum TcpConnectionStatus:uint8_t{
    TCP_STATUS_MIN,
    TCP_CONNECTING,
    TCP_CONNECTED,
    TCP_DISCONNECT,
};    
}
