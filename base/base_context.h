#pragma once
#include <utility>
#include <memory>
#include <deque>
#include <set>
#include "base_mutex.h"
#include "base_magic.h"
#include "epoll_api.h"
#include "base_clock.h"
#include "base_alarm_factory.h"
namespace basic{
class QueuedTask {
 public:
  QueuedTask() {}
  virtual ~QueuedTask() {}
  virtual bool Run() = 0;
};
template <class Closure>
class ClosureTask : public QueuedTask {
 public:
  explicit ClosureTask(Closure&& closure)
      : closure_(std::forward<Closure>(closure)) {}
 private:
  bool Run() override {
    closure_();
    return true;
  }

  typename std::remove_const<
      typename std::remove_reference<Closure>::type>::type closure_;
};
template <class Closure, class Cleanup>
class ClosureTaskWithCleanup : public ClosureTask<Closure> {
 public:
  ClosureTaskWithCleanup(Closure&& closure, Cleanup&& cleanup)
      : ClosureTask<Closure>(std::forward<Closure>(closure)),
        cleanup_(std::forward<Cleanup>(cleanup)) {}
  ~ClosureTaskWithCleanup() { cleanup_(); }

 private:
  typename std::remove_const<
      typename std::remove_reference<Cleanup>::type>::type cleanup_;
};

// Convenience function to construct closures that can be passed directly
// to methods that support std::unique_ptr<QueuedTask> but not template
// based parameters.
template <class Closure>
static std::unique_ptr<QueuedTask> NewClosure(Closure&& closure) {
  return std::make_unique<ClosureTask<Closure>>(std::forward<Closure>(closure));
}

template <class Closure, class Cleanup>
static std::unique_ptr<QueuedTask> NewClosure(Closure&& closure,
                                              Cleanup&& cleanup) {
  return std::make_unique<ClosureTaskWithCleanup<Closure, Cleanup>>(
      std::forward<Closure>(closure), std::forward<Cleanup>(cleanup));
}

class BaseContext{
public:
    class ExitVisitor{
    public:
        virtual ~ExitVisitor(){}
        virtual void ExitGracefully()=0;
    };
    BaseContext();
    virtual ~BaseContext();
    const QuicClock *clock() {return clock_.get();}
    BaseAlarmFactory* alarm_factory() {return alarm_factory_.get();}
    EpollServer* epoll_server() {return &epoll_server_;}
    template <class Closure,
          typename std::enable_if<!std::is_convertible<
              Closure,
              std::unique_ptr<QueuedTask>>::value>::type* = nullptr>
    void PostTask(Closure&& closure){
        PostInnerTask(NewClosure(std::forward<Closure>(closure)));
    }
    void HandleEvent();
    bool RegisterExitVisitor(ExitVisitor *visitor);
    bool UnRegisterExitVisitor(ExitVisitor *visitor);
protected:
    void PostInnerTask(std::unique_ptr<QueuedTask> task);
    void ExecuteTask();
    basic::EpollServer epoll_server_;
    std::unique_ptr<QuicClock> clock_;
    std::unique_ptr<BaseAlarmFactory> alarm_factory_;
    mutable basic::Mutex task_mutex_;
    std::deque<std::unique_ptr<basic::QueuedTask>>  queued_tasks_;
    std::set<ExitVisitor*> exit_visitors_;
};    
}