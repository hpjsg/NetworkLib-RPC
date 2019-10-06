#ifndef NET_CHANNEL_H
#define NET_CHANNEL_H

#include<functional>
#include<boost/noncopyable.hpp>
#include<sys/epoll.h>


class Eventloop; //前向声明
class Channel: boost::noncopyable
{
    public:
        typedef std::function<void()> Eventcallback;
        
        Channel(Eventloop* loop,int fd);
        ~Channel();
 
        void handleevent();
        void setreadcallback(const Eventcallback &cb)
        {readcallback_ = std::move(cb); }
        void setwritecallback(const Eventcallback &cb)
        {writecallback_ = std::move(cb); }
        void seterrorcallback(const Eventcallback &cb)
        {errorcallback_ = std::move(cb); }
        void setclosecallback(const Eventcallback &cb)
        {closecallback_ = std::move(cb); }

        int fd()const {return fd_;}
        __uint32_t events()const {return events_;}  
        void set_events(__uint32_t evt){events_ = evt;}
        void set_revents(__uint32_t revt){revents_ = revt;}


        void enablereading(){events_ |= EPOLLIN;} 
        void enablewriting(){events_ |= EPOLLOUT;}
        void disablereading() {events_ &= ~EPOLLIN;}
        void disablewriting(){events_ &= ~EPOLLOUT;}

        Eventloop* ownerloop(){return loop_;}

    private:

        static const int kreadevent;
        static const int kwriteevent;
        static const int knoneevent;

        Eventloop* loop_;
        int fd_;
        __uint32_t events_;
        __uint32_t revents_;


        bool inhandling_;

        Eventcallback readcallback_;
        Eventcallback writecallback_;
        Eventcallback errorcallback_;
        Eventcallback closecallback_;


}; 

#endif