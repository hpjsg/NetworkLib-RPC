#include"net/Timestamp.h"
#include<sys/time.h>

Timestamp::Timestamp()
    :microseconds_(0)
{
}

Timestamp::Timestamp(int64_t microseconds)
    :microseconds_(microseconds)
{
}

Timestamp Timestamp::now()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    int64_t seconds = tv.tv_sec;
    return Timestamp(seconds*Timestamp::secondstomicro+tv.tv_usec);
}
