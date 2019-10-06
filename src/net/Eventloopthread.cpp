#include<functional>
#include"net/Eventloopthread.h"
#include<stdio.h>

Eventloopthread::Eventloopthread()
    :loop_(NULL),
    exiting_(false),
    thread_(std::bind(&Eventloopthread::threadfunc,this),"Eventloopthread"),
    mutex_(),
    cond_(mutex_)
{
}

Eventloopthread::~Eventloopthread()
{
    exiting_ = true;
    if(loop_!=NULL)
    {
        loop_->quit();
        thread_.join();
    }
}
Eventloop* Eventloopthread::startloop()
{
    assert(!thread_.started());
    thread_.start();
    {
        MutexLockGuard lock(mutex_);
        while(loop_ == NULL)
        {
            cond_.wait();
        }
    }

    return loop_;
}

void Eventloopthread::threadfunc()
{
    Eventloop loop;
    {
        MutexLockGuard lock(mutex_);
        loop_ = &loop;
        cond_.notify();
    }

    loop.loop();
    //loop_ = NULL;?????
}