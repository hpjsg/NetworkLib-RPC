#ifndef NET_EVENTLOOP_H
#define NET_EVENTLOOP_H


#include<memory>
#include<boost/noncopyable.hpp>
#include<assert.h>
#include<functional>
#include<vector>
#include"base/Mutex.h"
#include"base/CurrentThread.h"
#include"net/Epoll.h"
#include"net/Timerqueue.h"

using namespace std;
class Epoll;
class Channel;
class Eventloop:boost::noncopyable
{
    public:
        typedef std::function<void()> Functor;
        typedef Timerqueue::Timerptr Timerptr;

        Eventloop();

        ~Eventloop();
        void loop();
        void quit();

        void runinloop(const Functor &&cb);
        void queueinloop(const Functor &&cb);

        void wakeup();//FIXME? private or public?
        bool isinloopthread()const {return threadId_ == CurrentThread::tid();}
        
        void assertinloopthread()
        {
            assert(isinloopthread());
        }

        void removechannel(shared_ptr<Channel> channel)
        {
            poller_->epoll_rmchannel(channel);
        }

        void updatechannel(shared_ptr<Channel> channel)
        {
            poller_->epoll_modchannel(channel);
        }

        void addchannel(shared_ptr<Channel> channel)
        {
            poller_->epoll_addchannel(channel);
        }

        Timerptr runevery(double interval,Timercallback cb);
        void cancel(Timerptr& time);

    private:
        bool looping_;
        const pid_t threadId_;
        shared_ptr<Epoll> poller_;
        bool quit_;
        int wakeupfd_;
        bool callingPendingFunctors_;//*atomic*???
        shared_ptr<Channel>wakeupchannel_;
        mutable MutexLock mutex_;
        vector<shared_ptr<Channel>>activechannels_;
        vector<Functor> pendingFunctors_;
        std::unique_ptr<Timerqueue> timerqueue_;

        void handleread();
        void dopendingFunctors();
        void handleconn();// FIXME

};

#endif