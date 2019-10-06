#include"net/Timer.h"

std::atomic_int64_t Timer::origin_id_(0);

Timer::Timer(const Timercallback& cb,Timestamp when,double interval)
    :timercallback_(cb),
    expiration_(when),
    interval_(interval),
    repeat_(interval_>0.0),
    id_(origin_id_.fetch_add(1))
{
    printf("timer ctor\n");
}