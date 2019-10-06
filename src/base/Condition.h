#ifndef BASE_CONDITION_H
#define BASE_CONDITION_H

#include<pthread.h>
#include<errno.h>
#include<cstdint>
#include<time.h>
#include<boost/noncopyable.hpp>
#include"base/Mutex.h"

class Condition: boost::noncopyable
{
    public:
        explicit Condition(MutexLock & mutex)
            : mutex_(mutex)
        {
            pthread_cond_init(&cond_,NULL);
        }

        ~Condition()
        {
            pthread_cond_destroy(&cond_);//someone on the conditon???
        }

        void wait()
        {
            pthread_cond_wait(&cond_,mutex_.get());
        }

        void notify()
        {
            pthread_cond_signal(&cond_);
        }

        void notifyall()
        {
            pthread_cond_broadcast(&cond_);
        }

    private:
        MutexLock &mutex_;
        pthread_cond_t cond_;


};

#endif