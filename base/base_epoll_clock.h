#pragma once
#include "base_time.h"
#include "base_clock.h"
#include "epoll_api.h"
#include "base_magic.h"
namespace basic{
class EpollClock : public QuicClock {
 public:
  explicit EpollClock(EpollServer* epoll_server);
  ~EpollClock() override;

  // Returns the approximate current time as a QuicTime object.
  QuicTime ApproximateNow() const override;

  // Returns the current time as a QuicTime object.
  // Note: this uses significant resources, please use only if needed.
  QuicTime Now() const override;

  // Returns the current time as a QuicWallTime object.
  // Note: this uses significant resources, please use only if needed.
  QuicWallTime WallNow() const override;

  // Override to do less work in this implementation.  The epoll clock is
  // already based on system (unix epoch) time, no conversion required.
  QuicTime ConvertWallTimeToQuicTime(
      const QuicWallTime& walltime) const override;

 protected:
  EpollServer* epoll_server_;
  // Largest time returned from Now() so far.
  mutable QuicTime largest_time_;

 private:
  BASE_DISALLOW_COPY_AND_ASSIGN(EpollClock);
};    
}