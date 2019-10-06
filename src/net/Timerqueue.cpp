#include"net/Timerqueue.h"
#include<sys/timerfd.h>
#include<string.h>
#include"net/Eventloop.h"

int createtimerfd()
{
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK|TFD_CLOEXEC);
    if(timerfd < 0)
    {
        printf("timerfd_create error\n");
    }
    return timerfd;
}

struct timespec howmuchtimefromnow(Timestamp when)
{
    int64_t microseconds = when.microseconds()-Timestamp::now().microseconds();
    microseconds = max((int64_t)100,microseconds); // why??
    struct timespec ts;
    ts.tv_sec = static_cast<__time_t>(microseconds/Timestamp::secondstomicro);
    ts.tv_nsec = static_cast<__syscall_slong_t>((microseconds%Timestamp::secondstomicro)*1000);

    return ts;
}

void resetTimerfd(int timerfd,Timestamp expiration)
{
    struct itimerspec newval;
    struct itimerspec oldval;
    bzero(&newval,sizeof(newval));
    newval.it_value = howmuchtimefromnow(expiration);
    int ret = timerfd_settime(timerfd,0,&newval,&oldval);
    if(ret)
    {
        printf("timerfd_settime error\n");
    }
}

void readTimerfd(int timerfd,Timestamp now)
{
    uint64_t count;
    read(timerfd,&count,sizeof(count));
    int64_t clock = now.microseconds()/Timestamp::secondstomicro;
    printf("%ld\n",clock);
}


Timerqueue::Timerqueue(Eventloop* loop)
    :loop_(loop),
    timerfd_(createtimerfd()),
    timerchannel_(new Channel(loop_,timerfd_)),
    timers_()
    {   
        printf("timerqueue ctor\n");
        timerchannel_->setreadcallback(std::bind(&Timerqueue::handleread,this));
        timerchannel_->set_events(EPOLLIN|EPOLLET);
        loop_->addchannel(timerchannel_);
    }

Timerqueue::~Timerqueue()
 {
     close(timerfd_);
 }

Timerqueue::Timerptr Timerqueue::addtimer(const Timercallback& cb,Timestamp when,double interval)
{
    Timerptr timer = make_shared<Timer>(cb,when,interval);
    loop_->runinloop(std::bind(&Timerqueue::addTimerinloop,this,timer));
    return timer;
}

void Timerqueue::cancel(Timerptr timer)
{
    loop_->runinloop(std::bind(&Timerqueue::canceltimerinloop,this,timer));
}

void Timerqueue::addTimerinloop(Timerptr& timer)
{
    loop_->assertinloopthread();
    bool earliestchanged = insert(timer);
    if(earliestchanged)
    {
        resetTimerfd(timerfd_,timer->expiration());
    }
}

void Timerqueue::canceltimerinloop(Timerptr& timer)
{
    Timerptr t(timer);
    timers_.erase({timer->expiration(),t});
}

bool Timerqueue::insert(Timerptr& timer)
{
    loop_->assertinloopthread();
    Timestamp deadline = timer->expiration();
    auto res = timers_.insert({deadline,timer});
    assert(res.second);
    if(res.first == timers_.begin())
        return true;
    else return false;
}

void Timerqueue::handleread()
{
    Timestamp now(Timestamp::now());
    readTimerfd(timerfd_,now);
    getExpired(now);
    for(auto iter = expired_.begin();iter !=expired_.end();iter++)
    {
        iter->second->run();
    }
    reset(now);
}

void Timerqueue::getExpired(Timestamp now)
{
    timeval sentry = std::make_pair(now,Timerptr());
    auto it = timers_.lower_bound(sentry);
    expired_.insert(expired_.end(),timers_.begin(),it);
    timers_.erase(timers_.begin(),it);
}

void Timerqueue::reset(Timestamp now)
{   
    for(auto it = expired_.begin();it != expired_.end();it++)
    {
        if(it->second->repeat())
        {
            it->second->restart(now);
            insert(it->second);
        }     
    }
    expired_.clear();
    if(!timers_.empty())
    {
        resetTimerfd(timerfd_,timers_.begin()->second->expiration());
    }
}


