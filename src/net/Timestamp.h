#ifndef NET_TIMESTAMP_H
#define NET_TIMESTAMP_H

#include<stdint.h>
#include<string>

class Timestamp
{
    public:
        Timestamp();
        explicit Timestamp(int64_t microseconds);
        static Timestamp now();

        int64_t microseconds()const{return microseconds_;}

        inline bool operator<(const Timestamp rhs)const
        {
            return microseconds_<rhs.microseconds_;
        }                // ==没必要重载吧

        static const int secondstomicro = 1000*1000;

        bool valid()const
        {return microseconds_>0;}

    private:
        int64_t microseconds_;

};

inline Timestamp addtime(Timestamp time,double seconds)
{
    int64_t delta = static_cast<int64_t>(seconds*Timestamp::secondstomicro);
    return Timestamp(time.microseconds()+delta);
}

inline Timestamp substractime(Timestamp time,double seconds)
{
    int64_t delta = static_cast<int64_t>(seconds*Timestamp::secondstomicro);
    return Timestamp(time.microseconds()-delta);
}

#endif