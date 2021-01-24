#pragma once
#include <chrono>
namespace basic{
inline int64_t WallTimeNowInUsec(){
    std::chrono::system_clock::duration d = std::chrono::system_clock::now().time_since_epoch();    
    std::chrono::microseconds mic = std::chrono::duration_cast<std::chrono::microseconds>(d);
    return mic.count(); 
}
inline int64_t TimeMillis(){
    return WallTimeNowInUsec()/1000;
}  
}
