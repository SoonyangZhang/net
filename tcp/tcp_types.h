#pragma once
#include <stdint.h>
namespace basic{
enum TcpConnectionStatus:uint8_t{
    CONNECTING,
    CONNECTED,
    DISCONNECT,
};    
}