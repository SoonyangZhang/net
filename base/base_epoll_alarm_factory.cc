#include <type_traits>
#include <memory>
#include "epoll_api.h"
#include "base_epoll_alarm_factory.h"

#include "logging.h"

namespace basic {
namespace {

class QuicEpollAlarm : public BaseAlarm {
 public:
  QuicEpollAlarm(EpollServer* epoll_server,
                 std::unique_ptr<BaseAlarm::Delegate> delegate)
      : BaseAlarm(std::move(delegate)),
        epoll_server_(epoll_server),
        epoll_alarm_impl_(this) {}

 protected:
  void SetImpl() override {
    DCHECK(deadline().IsInitialized());
    epoll_server_->RegisterAlarm(
        (deadline() - QuicTime::Zero()).ToMicroseconds(), &epoll_alarm_impl_);
  }

  void CancelImpl() override {
    DCHECK(!deadline().IsInitialized());
    epoll_alarm_impl_.UnregisterIfRegistered();
  }

  void UpdateImpl() override {
    DCHECK(deadline().IsInitialized());
    int64_t epoll_deadline = (deadline() - QuicTime::Zero()).ToMicroseconds();
    if (epoll_alarm_impl_.registered()) {
      epoll_alarm_impl_.ReregisterAlarm(epoll_deadline);
    } else {
      epoll_server_->RegisterAlarm(epoll_deadline, &epoll_alarm_impl_);
    }
  }

 private:
  class EpollAlarmImpl : public EpollAlarmBase {
   public:
    using int64_epoll = decltype(EpollAlarmBase().OnAlarm());

    explicit EpollAlarmImpl(QuicEpollAlarm* alarm) : alarm_(alarm) {}

    // Use the same integer type as the base class.
    int64_epoll OnAlarm() override {
      EpollAlarmBase::OnAlarm();
      alarm_->Fire();
      // Fire will take care of registering the alarm, if needed.
      return 0;
    }

   private:
    QuicEpollAlarm* alarm_;
  };

  EpollServer* epoll_server_;
  EpollAlarmImpl epoll_alarm_impl_;
};

}  // namespace

BaseEpollAlarmFactory::BaseEpollAlarmFactory(EpollServer* epoll_server)
    : epoll_server_(epoll_server) {}

BaseEpollAlarmFactory::~BaseEpollAlarmFactory() = default;

BaseAlarm* BaseEpollAlarmFactory::CreateAlarm(BaseAlarm::Delegate* delegate) {
  return new QuicEpollAlarm(epoll_server_,
                            std::unique_ptr<BaseAlarm::Delegate>(delegate));
}
}  // namespace quic
