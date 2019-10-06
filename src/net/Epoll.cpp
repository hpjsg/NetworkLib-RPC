#include"net/Epoll.h"
#include<assert.h>
#include<sys/epoll.h>
#include<errno.h>
#include<memory>
#include"net/Channel.h"
#include"net/Eventloop.h"
using namespace std;

const int EVENTS_NUM = 4096;
const int EPOLLWAITTIME = 10000;

Epoll::Epoll(Eventloop*loop)
    :ownerloop_(loop),
    epollfd_(epoll_create1(EPOLL_CLOEXEC)),//USING 1!!!
    events_(EVENTS_NUM)
{
    assert(epollfd_>0);       
}

Epoll::~Epoll()
{
    close(epollfd_);
}

void Epoll::epoll_addchannel(shared_ptr<Channel> req)
{
    int fd = req->fd();
    fd2channel_[fd] = req;
    struct epoll_event event;
    event.data.fd = fd;
    event.events = req->events();

    if(epoll_ctl(epollfd_,EPOLL_CTL_ADD,fd,&event)<0)
    {
        perror("epoll_add error");
        fd2channel_[fd].reset();
    }
}

void Epoll::epoll_modchannel(shared_ptr<Channel> req)
{
    int fd = req->fd();
    struct epoll_event event;
    event.data.fd = fd;
    event.events = req->events();
    if(epoll_ctl(epollfd_,EPOLL_CTL_MOD,fd,&event)<0)
    {
        perror("epoll_mod error");
        fd2channel_[fd].reset();
    }   
}

void Epoll::epoll_rmchannel(shared_ptr<Channel> req)
{
    int fd = req->fd();
    struct epoll_event event;
    event.data.fd = fd;
    event.events = req->events();
    if(epoll_ctl(epollfd_,EPOLL_CTL_DEL,fd,&event)<0)
    {
        perror("epoll_mod error");
    }
    fd2channel_[fd].reset();
}

std::vector<shared_ptr<Channel>> Epoll::poll()
{
    while(true)
    {
        int numevents = epoll_wait(epollfd_,&*events_.begin(),events_.size(),EPOLLWAITTIME);
        if(numevents<0) 
            //perror("epoll wait error");
            continue;
        std::vector<shared_ptr<Channel>>ret = getactivechannel(numevents);
        if(ret.size() > 0)
            return ret; 
    }                  
}

std::vector<shared_ptr<Channel>>Epoll::getactivechannel(int numevents)
{
    std::vector<shared_ptr<Channel>>ret;
    for(int i=0;i<numevents;i++)
    {
        int fd = events_[i].data.fd;
        shared_ptr<Channel> cur_req = fd2channel_[fd];

        if(cur_req)
        {
            cur_req->set_revents(events_[i].events);
            //cur_req->set_events(0);
            ret.push_back(cur_req);
        }
        else
        {

        }
    }
    return ret;
}