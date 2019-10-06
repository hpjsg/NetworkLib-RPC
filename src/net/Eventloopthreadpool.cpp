#include"net/Eventloopthreadpool.h"
#include"net/Eventloop.h"
#include<assert.h>
#include <stdio.h>
Eventloopthreadpool::Eventloopthreadpool(Eventloop*baseloop,int numthreads)
    : baseloop_(baseloop),
    started_(false),
    numthreads_(numthreads),
    next_(0)
{
}

Eventloopthreadpool::~Eventloopthreadpool()
{   
}

void Eventloopthreadpool::start()
{
    assert(!started_);
    baseloop_->assertinloopthread();

    started_ = true;
    for(int i = 0;i < numthreads_; i++)
    {
        Eventloopthread* t = new Eventloopthread;
        threads_.push_back(t);
        loops_.push_back(t->startloop());
    }
}

Eventloop* Eventloopthreadpool::getnextloop()
{
    baseloop_->assertinloopthread();
    Eventloop* loop = baseloop_;

    if(!loops_.empty())
    {
        loop = loops_[next_];
        ++next_;
        if(static_cast<size_t>(next_) >= loops_.size())
        {
            next_ = 0;
        }
    }
    return loop;
}