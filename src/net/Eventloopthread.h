#ifndef NET_EVENTLOOPTHREAD_H
#define NET_EVENTLOOPTHREAD_H

#include<boost/noncopyable.hpp>
#include"base/Condition.h"
#include"base/Mutex.h"
#include"base/Thread.h"
#include"net/Eventloop.h"

class Eventloopthread: boost::noncopyable
{
    public:
        Eventloopthread();
        ~Eventloopthread();
        Eventloop* startloop();
    private:
        void threadfunc();
        
        Eventloop *loop_;
        bool exiting_;
        Thread thread_;
        MutexLock mutex_;
        Condition cond_;
};

#endif