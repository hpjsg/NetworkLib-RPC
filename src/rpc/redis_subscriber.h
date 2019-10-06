#ifndef RPC_REDIS_SUBSCRIBER_H
#define RPC_REDIS_SUBSCRIBER_H

#include <stdlib.h>
#include <string>
#include <functional>
#include <map>
#include <set>
#include<vector>
#include <hiredis/async.h>
#include"net/Eventloop.h"
#include "base/Mutex.h"
#include "rpc/RedisConnectionpool.h"
#include "rpc/hiredis_adapter.h"


class CRedisSubscriber
{
public:

    bool init();

    // 可以多次调用，订阅多个频道
    bool subscribe(const std::string channel_name);
    bool unsubscribe(const std::string channel_name);
    vector<string> inquiryandrefresh(const std::string channel_name);
    vector<string> discovery(const std::string channel_name);

    static CRedisSubscriber* getinstance()
    {return subscriber_;}

    static void setloop(Eventloop* loop){loop_ = loop;}

    static void setip(std::string ip){ip_ = ip;}

    static void setport(uint16_t port){port_ = port;}
    
    const int connection_num = 10;

private:

    CRedisSubscriber();
    ~CRedisSubscriber();

    // 执行命令回调
    static void subscribe_callback(redisAsyncContext *redis_context,
        void *reply, void *privdata);


private:

    static Eventloop* loop_;

    static std::string ip_;

    static uint16_t port_;

    static map<string,set<string>> cache;
    // 通知外层的回调函数对象

    static MutexLock mutex_;

    static CRedisSubscriber* subscriber_;

    static RedisConnectionpool* conn_;
};

#endif