#ifndef BASE_MUTEX_H
#define BASE_MUTEX_H

#include<pthread.h>
#include<cstdio>
#include<boost/noncopyable.hpp>

class MutexLock: boost::noncopyable
{
    public:
        MutexLock()
        {
            pthread_mutex_init(&mutex,NULL);
        }
        ~MutexLock()
        {
            pthread_mutex_lock(&mutex);
            pthread_mutex_destroy(&mutex);
        }
        void lock()
        {
            pthread_mutex_lock(&mutex);
        }
        void unlock()
        {
            pthread_mutex_unlock(&mutex);
        }
        pthread_mutex_t *get()//?????
        {
            return &mutex;
        }
    private:
        pthread_mutex_t mutex;
        //friend class Condition;useless?
};

class MutexLockGuard: boost::noncopyable
{
    public:
        explicit MutexLockGuard(MutexLock &mutex_)
            :mutex(mutex_)
        {
            mutex.lock();
        }
        ~MutexLockGuard()
        {
            mutex.unlock();
        }
    private:
        MutexLock&  mutex;
};

#endif