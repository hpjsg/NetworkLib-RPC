#ifndef NET_EPOLL_H
#define NET_EPOLL_H

#include<vector>
#include<sys/epoll.h>
#include <memory>
#include"net/Channel.h"
using namespace std;
class Eventloop;
class Epoll
{
    public:
        Epoll(Eventloop* loop);
        ~Epoll();
     
        //void updatechannel(Channel* channel);
        void epoll_modchannel(shared_ptr<Channel> req);
        void epoll_addchannel(shared_ptr<Channel> req);
        void epoll_rmchannel(shared_ptr<Channel> req);

        std::vector<std::shared_ptr<Channel>> poll();
        std::vector<std::shared_ptr<Channel>>getactivechannel(int events_num); 
        //void assertinloopthread(){ownerloop_->assertinloopthread();}

        int getEpollfd()
        {
            return epollfd_;
        }
    private:
        
        static const int MAXFDS = 100000;
        std::shared_ptr<Channel> fd2channel_[MAXFDS];
        Eventloop* ownerloop_;
        int epollfd_;
        std::vector<struct epoll_event>events_;
};

#endif