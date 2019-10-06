#include "rpc/redis_publisher.h"
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include "net/Timestamp.h"

std::string CRedisPublisher::ip_ = "127.0.0.1";
uint16_t CRedisPublisher::port_ = 9981;
std::map<std::string,Timerqueue::Timerptr> CRedisPublisher::Timermap;
MutexLock CRedisPublisher::mutex_;
CRedisPublisher* CRedisPublisher::publisher_ = new CRedisPublisher(); 
Eventloop* CRedisPublisher::loop_ = nullptr;
RedisConnectionpool* CRedisPublisher::conn_ = new RedisConnectionpool(CRedisPublisher::publisher_->connection_num);

CRedisPublisher::CRedisPublisher()
{
}

CRedisPublisher::~CRedisPublisher()
{
}

bool CRedisPublisher::init()
{
    if(loop_ == nullptr)
    {
        printf("reactor has not been set\n");
        return false;
    }
    else
    {
        conn_->init(loop_,ip_,port_);
    }
    return true;
}



bool CRedisPublisher::publish(const std::string key,
    const std::string val)
{
    redisAsyncContext * _redis_context =  conn_->getasynccontext();
    int ret = redisAsyncCommand(_redis_context, 
        CRedisPublisher::command_callback, this, "PUBLISH %s %s", 
        key.c_str(), val.c_str());

    conn_->returnasynccontext(_redis_context);
    if (REDIS_ERR == ret)
    {
        printf("Publish command failed: %d\n", ret);
        return false;
    }
    printf("Publish command success\n");
    return true;
}

bool CRedisPublisher::registration(const std::string key,const std::string val)
{
    Timestamp now = Timestamp::now();
    redisAsyncContext * _redis_context =  conn_->getasynccontext();
    int ret = redisAsyncCommand(_redis_context,nullptr,this,"ZADD %s %ld %s",key.c_str(),now,val.c_str());
    conn_->returnasynccontext(_redis_context);
    if (REDIS_ERR == ret)
    {
        printf(" register failed: %d\n", ret);
        return false;
    }
    printf(" register successs\n");
    Timerptr timer = loop_->runevery(10.0,std::bind(&CRedisPublisher::refresh,this,key,val));
    {
        MutexLockGuard lock(mutex_);
        Timermap[key+val] = timer;
    }
    if(publish(key,"Register:"+val))
        return true;
    else 
        return false;
}

void CRedisPublisher::refresh(const std::string key,const std::string val)
{
    Timestamp now = Timestamp::now();
    redisAsyncContext * _redis_context =  conn_->getasynccontext();
    int ret = redisAsyncCommand(_redis_context,nullptr,this,"ZADD %s %ld %s",key.c_str(),now,val.c_str());
    conn_->returnasynccontext(_redis_context);
    if (REDIS_ERR == ret)
    {
        printf(" refresh failed: %d\n", ret);

    }
    printf(" refresh successs\n");   
}

bool CRedisPublisher::unregistration(const std::string key,const std::string val)
{
    Timerptr timer;
    {
        MutexLockGuard lock(mutex_);
        auto it = Timermap.find(key+val);
        if(it != Timermap.end())
        {
            timer = it->second;
            Timermap.erase(it);
        }
    }
    loop_->cancel(timer);
    redisAsyncContext * _redis_context =  conn_->getasynccontext();
    int ret = redisAsyncCommand(_redis_context,nullptr,nullptr,"ZREM %s %s",key.c_str(),val.c_str());
    conn_->returnasynccontext(_redis_context);
    if (REDIS_ERR == ret)
    {
        printf(" unregister failed: %d\n", ret);
        return false;
    }
    printf(" unregister successs\n");

    if(publish(key,"unRegister:"+val))                // 需要将之前的runevery 注销掉
        return true;
    else 
        return false;
}


void CRedisPublisher::command_callback(redisAsyncContext *redis_context,
    void *reply, void *privdata)
{
    printf("command callback.\n");
}

