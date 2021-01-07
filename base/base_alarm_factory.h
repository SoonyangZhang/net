#pragma once
#include "base_alarm.h"
namespace basic{
class  BaseAlarmFactory {
 public:
  virtual ~BaseAlarmFactory() {}
  virtual BaseAlarm* CreateAlarm(BaseAlarm::Delegate* delegate) = 0;
};    
}