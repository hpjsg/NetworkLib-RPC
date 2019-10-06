#ifndef BASE_COUNTDOWNLATCH_H
#define BASE_COUNTDOWNLATCH_H

#include<boost/noncopyable.hpp>
#include"base/Condition.h"
#include"base/Mutex.h"


class CountDownLatch: boost::noncopyable
{
    public:
        explicit CountDownLatch(int count);
        void wait();
        void countdown();

    private:
        mutable MutexLock mutex_;
        Condition condition_;
        int count_;
};
#endif