#ifndef NET_TIMER_H
#define NET_TIMER_H

#include<boost/noncopyable.hpp>
#include<functional>
#include<atomic>
#include"net/Timestamp.h"


typedef std::function<void()> Timercallback;//FIXME

class Timer: boost::noncopyable
{
    public:
        Timer(const Timercallback& cb,Timestamp when,double interval);

        ~Timer(){};
        
        void run()const
        {
            timercallback_();
        }

        Timestamp expiration()const
        {return expiration_;}

        bool repeat()const
        {return repeat_;}

        int64_t id()
        {return id_;}

        void restart(Timestamp now)
        {
            expiration_ = addtime(now,interval_);
        }

    private:
        const Timercallback timercallback_;
        Timestamp expiration_;
        const double interval_;
        const bool repeat_;
        const int64_t id_;
        static std::atomic_int64_t origin_id_;
};

#endif

