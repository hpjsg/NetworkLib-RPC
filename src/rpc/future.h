#ifndef RPC_FUTURE_H
#define RPC_FUTURE_H

#include"base/Condition.h"
#include"base/Mutex.h"
template<typename RESPONSE>
class future
{   
    public:
    
        explicit future()       
            :put_(false),mutex_(),cond_(mutex_)
        {}

        RESPONSE& get()
        {
            MutexLockGuard lock(mutex_);
            while(!put_)
            {
                cond_.wait();
            }
            printf("get the response");
            return response_;
        }

        void push(RESPONSE& res)
        {
            MutexLockGuard lock(mutex_);
            response_ = res;
            put_ = true;
            printf("push in:");
            cond_.notify();
        }

        void inspect()
        {
            printf("in the inspect\n");
            printf("%s\n",get()->payload().c_str());
        }
        

    private:
        RESPONSE response_;   
        volatile bool put_;
        mutable MutexLock mutex_;
        Condition cond_;
};

#endif
