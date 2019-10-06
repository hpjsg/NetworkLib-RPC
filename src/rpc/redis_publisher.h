#ifndef RPC_REDIS_PUBLISHER_H
#define RPC_REDIS_PUBLISHER_H

#include <stdlib.h>
#include <hiredis/async.h>
#include <unistd.h>
#include <map>
#include<functional>
#include"net/Eventloop.h"
#include"net/Channel.h"
#include "net/Timer.h"    
#include "rpc/RedisConnectionpool.h"
#include"rpc/hiredis_adapter.h"

class CRedisPublisher
{
    public:
        typedef Timerqueue::Timerptr Timerptr;    

        bool init();

        bool publish(const std::string channel_name, 
            const std::string message);

        bool registration(const std::string key,const std::string val);
        bool unregistration(const std::string key, const std::string val);
        void refresh(const std::string key,const std::string val);

        static void setloop(Eventloop* loop){loop_ = loop;}       
        static void setip(std::string ip){ip_ = ip;}
        static void setport(uint16_t port){port_ = port;}
    
        const int connection_num = 5;

        static CRedisPublisher* getinstance(){return publisher_;}
    private:

        static void command_callback(redisAsyncContext *redis_context,
            void *reply, void *privdata);

    private:

        CRedisPublisher();
        ~CRedisPublisher();

        static Eventloop* loop_;

        static std::map<std::string,Timerptr> Timermap;
        static MutexLock mutex_;

        static CRedisPublisher* publisher_;

        static std::string ip_;
        static uint16_t port_;

        static RedisConnectionpool* conn_;
};
#endif