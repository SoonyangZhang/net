#include "base_epoll_clock.h"
namespace basic{
EpollClock::EpollClock(EpollServer* epoll_server)
    : epoll_server_(epoll_server), largest_time_(QuicTime::Zero()) {}

EpollClock::~EpollClock() {}

QuicTime EpollClock::ApproximateNow() const {
  return CreateTimeFromMicroseconds(epoll_server_->ApproximateNowInUsec());
}

QuicTime EpollClock::Now() const {
  QuicTime now = CreateTimeFromMicroseconds(epoll_server_->NowInUsec());

  if (now <= largest_time_) {
    // Time not increasing, return |largest_time_|.
    return largest_time_;
  }

  largest_time_ = now;
  return largest_time_;
}

QuicWallTime EpollClock::WallNow() const {
  return QuicWallTime::FromUNIXMicroseconds(
      epoll_server_->ApproximateNowInUsec());
}

QuicTime EpollClock::ConvertWallTimeToQuicTime(
    const QuicWallTime& walltime) const {
  return QuicTime::Zero() +
         QuicTime::Delta::FromMicroseconds(walltime.ToUNIXMicroseconds());
}    
}