#pragma once
#include "logging.h"
#define EPOLL_BUG_IMPL LOG(FATAL)
#define EPOLL_BUG EPOLL_BUG_IMPL