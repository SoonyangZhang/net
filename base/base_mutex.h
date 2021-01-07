#pragma once
#include <pthread.h>
namespace basic{
class  MutexImpl final {
public:
    MutexImpl() {
        pthread_mutexattr_t mutex_attribute;
        pthread_mutexattr_init(&mutex_attribute);
        pthread_mutex_init(&mutex_, &mutex_attribute);
        pthread_mutexattr_destroy(&mutex_attribute);
    }
    MutexImpl(const MutexImpl&) = delete;
    MutexImpl& operator=(const MutexImpl&) = delete;
    ~MutexImpl() { pthread_mutex_destroy(&mutex_); }
    
    void Lock()  { pthread_mutex_lock(&mutex_); }
    bool TryLock() {
    return pthread_mutex_trylock(&mutex_) == 0;
    }
    void Unlock(){ pthread_mutex_unlock(&mutex_); }
 private:
    pthread_mutex_t mutex_;
}; 
class  Mutex final {
public:
    Mutex() = default;
    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;
    
    void Lock()  {
        impl_.Lock();
    }
    bool TryLock()  {
        return impl_.TryLock();
    }
    void Unlock(){
        impl_.Unlock();
    }
 private:
    MutexImpl impl_;
};

// MutexLock, for serializing execution through a scope.
class  MutexLock final {
public:
    MutexLock(const MutexLock&) = delete;
    MutexLock& operator=(const MutexLock&) = delete;
    
    explicit MutexLock(Mutex* mutex): mutex_(mutex) {
        mutex->Lock();
    }
    ~MutexLock() { mutex_->Unlock(); }

private:
    Mutex* mutex_;
};
}
