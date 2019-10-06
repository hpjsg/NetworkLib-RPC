#include"net/Eventloop.h"
#include"net/Channel.h"
#include<iostream>
#include<signal.h>
#include<sys/epoll.h>
#include<sys/eventfd.h>
#include"base/Thread.h"
#include"base/CurrentThread.h"

using namespace std;

__thread  Eventloop* t_loopInThisThread = 0;// _and_

int createeventfd()
{
    int evtfd = ::eventfd(0,EFD_NONBLOCK | EFD_CLOEXEC);
    if(evtfd < 0)
    {
        abort();
    }
    return evtfd;
}

class Ignoresigpipe
{
    public:
        Ignoresigpipe()
        {
            signal(SIGPIPE,SIG_IGN);
        }

};

Eventloop::Eventloop()
    :looping_(false),
    threadId_(CurrentThread::tid()),
    poller_(new Epoll(this)),
    quit_(false),
    wakeupfd_(createeventfd()),
    callingPendingFunctors_(false),
    wakeupchannel_(new Channel(this,wakeupfd_)),
    timerqueue_(new Timerqueue(this))
{
    if(t_loopInThisThread)
    {

    }
    else
    {
        t_loopInThisThread = this;
    }
    wakeupchannel_->set_events(EPOLLIN|EPOLLET);//Edge trigger
    wakeupchannel_->setreadcallback(bind(&Eventloop::handleread,this));
    poller_->epoll_addchannel(wakeupchannel_);
}

Eventloop::~Eventloop()
{
    assert(!looping_);
    close(wakeupfd_);
    t_loopInThisThread = 0;
}

void Eventloop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = write(wakeupfd_,&one,sizeof(one));
    if(n != sizeof one)
    {

    }
}

void Eventloop::handleread()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupfd_,&one,sizeof one);
     if(n != sizeof one)
    {

    }
    //wakeupchannel_->set_events(EPOLLIN | EPOLLET);???
}

void Eventloop::quit()
{
    quit_ = true;
    if(!isinloopthread())
    {
        wakeup();
    }
}

void Eventloop::runinloop(const Functor&& cb)
{
    if(isinloopthread())
    {
        cb();
    }
    else
    {
        queueinloop(std::move(cb));
    }
}

void Eventloop::queueinloop(const Functor&& cb)
{
    {
        MutexLockGuard lock(mutex_);
        pendingFunctors_.emplace_back(std::move(cb));
    }
    if(!isinloopthread()||callingPendingFunctors_)
    {
        wakeup();
    }
}

void Eventloop::dopendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        MutexLockGuard lock(mutex_);
        functors.swap(pendingFunctors_);

    }
    for(size_t i = 0;i < functors.size();i++)
    {
        functors[i]();
    }
    callingPendingFunctors_ = false;
}

void Eventloop::loop()
{
    assert(!looping_);
    assert(isinloopthread());
    looping_ = true;
    quit_ = false;

    while(!quit_)
    {
        activechannels_.clear();
        activechannels_ = poller_->poll();
        for(auto &it:activechannels_)
            it->handleevent();
        dopendingFunctors();
    }
    looping_ = false;

}

Timerqueue::Timerptr Eventloop::runevery(double interval,Timercallback cb)
{
    Timestamp time(addtime(Timestamp::now(),interval));
    return timerqueue_->addtimer(std::move(cb),time,interval); //??第一个不是个临时变量吗？？？   
}

void Eventloop::cancel(Timerptr& timer)
{
    return timerqueue_->cancel(timer);
}