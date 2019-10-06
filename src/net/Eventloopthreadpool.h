#ifndef NET_EVENTLOOPTHREADPOOL_H
#define NET_EVENTLOOPTHREADPOOL_H


#include<boost/noncopyable.hpp>
#include<memory>
#include<vector>
#include <boost/ptr_container/ptr_vector.hpp>
#include"net/Eventloopthread.h"

class Eventloopthreadpool: boost::noncopyable
{
    public:
        Eventloopthreadpool(Eventloop* baseloop,int numthreads);

        ~Eventloopthreadpool();

        void start();

        Eventloop* getnextloop();

    private:
        Eventloop* baseloop_;
        bool started_;
        int numthreads_;
        int next_;
        boost::ptr_vector<Eventloopthread> threads_;
        std::vector<Eventloop*> loops_;
};

#endif