set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(${CMAKE_SOURCE_DIR}/)
set(base_LIB
${CMAKE_SOURCE_DIR}/base/base_alarm.cc
${CMAKE_SOURCE_DIR}/base/base_clock.cc
${CMAKE_SOURCE_DIR}/base/base_context.cc
${CMAKE_SOURCE_DIR}/base/base_epoll_alarm_factory.cc  
${CMAKE_SOURCE_DIR}/base/base_epoll_clock.cc
${CMAKE_SOURCE_DIR}/base/base_ini.c
${CMAKE_SOURCE_DIR}/base/base_thread.cc
${CMAKE_SOURCE_DIR}/base/base_time.cc
${CMAKE_SOURCE_DIR}/base/byte_codec.cc 
${CMAKE_SOURCE_DIR}/base/epoll_thread.cc
${CMAKE_SOURCE_DIR}/base/file_op.cc
${CMAKE_SOURCE_DIR}/base/ip_address.cc
${CMAKE_SOURCE_DIR}/base/random.cc
${CMAKE_SOURCE_DIR}/base/simple_epoll_server.cc
${CMAKE_SOURCE_DIR}/base/socket_address.cc
${CMAKE_SOURCE_DIR}/tcp/tcp_client.cc
${CMAKE_SOURCE_DIR}/tcp/tcp_handle.cc
${CMAKE_SOURCE_DIR}/tcp/tcp_server.cc
)
add_library(base STATIC ${base_LIB})
