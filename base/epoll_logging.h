#pragma once
#include "logging.h"
namespace basic{
/*
#define EPOLL_LOG_IMPL(severity) DLOG(severity)
#define EPOLL_VLOG_IMPL(verbose_level) DLOG(INFO)
#define EPOLL_DVLOG_IMPL(verbose_level) DLOG(INFO)
#define EPOLL_PLOG_IMPL(severity) DLOG(severity)
*/
#define EPOLL_LOG_IMPL(severity)  DUMMY_LOG(severity)
#define EPOLL_VLOG_IMPL(verbose_level)  DUMMY_LOG(INFO)
#define EPOLL_DVLOG_IMPL(verbose_level) DUMMY_LOG(INFO)
#define EPOLL_PLOG_IMPL(severity)  DUMMY_LOG(severity)

#define EPOLL_LOG(severity) EPOLL_LOG_IMPL(severity)
#define EPOLL_VLOG(verbosity) EPOLL_VLOG_IMPL(INFO)
#define EPOLL_DVLOG(verbosity) EPOLL_DVLOG_IMPL(INFO)
#define EPOLL_PLOG(severity) EPOLL_PLOG_IMPL(severity)
}
