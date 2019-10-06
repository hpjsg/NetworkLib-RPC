#include"net/Channel.h"
#include"net/Eventloop.h"
#include<sys/epoll.h>
#include<assert.h>



Channel::Channel(Eventloop *loop,int fdarg)
    :loop_(loop),
    fd_(fdarg),
    events_(0),
    revents_(0),
    inhandling_(false)
    {
    }
Channel::~Channel()
{
    assert(!inhandling_);
}


void Channel::handleevent(){
    inhandling_ = true;
    if((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
    {
        if(closecallback_) closecallback_();
    }
    if(revents_ & EPOLLERR)
    {
        if(errorcallback_) errorcallback_();
    }
    if(revents_& (EPOLLIN|EPOLLPRI|EPOLLRDHUP))
    {
        readcallback_();
    }
    if(revents_& EPOLLOUT)
    {
        writecallback_();
    }
    inhandling_ = false;
}