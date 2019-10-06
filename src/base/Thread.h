#ifndef BASE_THREAD_H
#define BASE_THREAD_H

#include<functional>
#include<memory>
#include<pthread.h>
#include<string>
#include<sys/syscall.h>
#include<unistd.h>
#include<boost/noncopyable.hpp>
#include "base/CountDownLatch.h"


class Thread : boost::noncopyable
{
    public:
        typedef std::function<void()> ThreadFunc;
        explicit Thread(const ThreadFunc&,const std::string&name = std::string());
        ~Thread();
        void start();
        int join();
        bool started()const{return started_;}
        pid_t tid()const {return tid_;}
        const std::string& name() const {return name_;}

    private:
        void setdefaultname();
        bool started_;
        bool joined_;
        pthread_t pthreadId_;
        pid_t tid_;
        ThreadFunc functor_;
        std::string name_;
        CountDownLatch latch_;


};

#endif
