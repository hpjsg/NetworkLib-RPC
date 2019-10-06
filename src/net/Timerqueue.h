#ifndef NET_TIMERQUEUE_H
#define NET_TIMERQUEUE_H


#include<set>
#include<vector>
#include<boost/noncopyable.hpp>
#include<memory>
#include"net/Timer.h"
#include"net/Channel.h"


class Timerqueue: boost::noncopyable       //timer* 换成unique_ptr
{
    public:
        typedef std::shared_ptr<Timer> Timerptr;
        typedef std::pair<Timestamp,Timerptr> timeval;
        Timerqueue(Eventloop* loop);
        ~Timerqueue();

        Timerptr addtimer(const Timercallback& cb,Timestamp when,double interval);
        void cancel(Timerptr timer);
    private:

        typedef std::set<timeval> Timerlist;

        void addTimerinloop(Timerptr& timer);
        void canceltimerinloop(Timerptr& timer);

        void handleread();
        void getExpired(Timestamp now);
        void reset(Timestamp now);

        bool insert(Timerptr& timer);

        Eventloop* loop_;
        const int timerfd_;
        std::shared_ptr<Channel>timerchannel_;
        Timerlist timers_;
        std::vector<timeval>expired_;

};

#endif