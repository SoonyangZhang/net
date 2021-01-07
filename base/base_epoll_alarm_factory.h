#pragma once
#include "base_alarm.h"
#include "base_alarm_factory.h"
#include "epoll_api.h"
namespace basic{
class BaseEpollAlarmFactory : public BaseAlarmFactory {
 public:
  explicit BaseEpollAlarmFactory(EpollServer* eps);
  BaseEpollAlarmFactory(const BaseEpollAlarmFactory&) = delete;
  BaseEpollAlarmFactory& operator=(const BaseEpollAlarmFactory&) = delete;
  ~BaseEpollAlarmFactory() override;

  // QuicAlarmFactory interface.
  BaseAlarm* CreateAlarm(BaseAlarm::Delegate* delegate) override;
 private:
  EpollServer* epoll_server_;  // Not owned.
};    
}
