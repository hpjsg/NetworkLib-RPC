#include "rpc/redis_subscriber.h"
#include <stddef.h>
#include <assert.h>
#include <string.h>


Eventloop* CRedisSubscriber::loop_ = nullptr;
std::string CRedisSubscriber::ip_ = "127.0.0.1";
uint16_t CRedisSubscriber::port_ = 9981;
map<string,set<string>> CRedisSubscriber::cache = map<string,set<string>>();//
MutexLock CRedisSubscriber::mutex_;
CRedisSubscriber* CRedisSubscriber::subscriber_ = new CRedisSubscriber();
RedisConnectionpool* CRedisSubscriber::conn_ = new RedisConnectionpool(CRedisSubscriber::subscriber_->connection_num);



CRedisSubscriber::CRedisSubscriber()
{
}

CRedisSubscriber::~CRedisSubscriber()
{
}

bool CRedisSubscriber::init()
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


/*bool CRedisSubscriber::disconnect()
{
    if (_redis_context)
    {
        redisAsyncDisconnect(_redis_context);
        redisAsyncFree(_redis_context);
        _redis_context = NULL;
    }

    return true;
}*/

bool CRedisSubscriber::subscribe(const std::string channel_name)
{
    redisAsyncContext * _redis_context =  conn_->getasynccontext();
    int ret = redisAsyncCommand(_redis_context, &CRedisSubscriber::subscribe_callback, this, "SUBSCRIBE %s", channel_name.c_str());
    conn_->returnasynccontext(_redis_context);
    if (REDIS_ERR == ret)
    {
        printf("Subscribe command failed: %d\n", ret);
        return false;
    }
    printf(": Subscribe success: %s\n", channel_name.c_str());


    return true;
}

bool CRedisSubscriber::unsubscribe(const std::string channel_name)
{
    redisAsyncContext * _redis_context =  conn_->getasynccontext();
    int ret = redisAsyncCommand(_redis_context,
    nullptr,nullptr,"UNSUBSCRIBE %s",channel_name.c_str());
    if(REDIS_ERR == ret)
    {
        printf("unSubscribe command failed: %d\n",ret);
        return false;
    }
    printf(": unSubscribe success: %s\n", channel_name.c_str());
    conn_->returnasynccontext(_redis_context);

    return true;
}

vector<string> CRedisSubscriber::inquiryandrefresh(const std::string channel_name)
{
    redisContext * context_ =  conn_->getsynccontext();
    printf("inquiry from redis\n");
    
    redisReply* redis_reply = reinterpret_cast<redisReply*>
        (redisCommand(context_, "ZRANGE %s 0 -1",channel_name.c_str()));

    conn_->returnsynccontext(context_);

    vector<string> host;
    set<string> update;

    if (NULL == redis_reply) 
    {
        return host;
    }
    int num = redis_reply->elements;
    if(redis_reply->type == REDIS_REPLY_ARRAY)
    {
        for(int i=0;i<num;i++)
        {
            string str = redis_reply->element[i]->str;
            host.push_back(str);
            update.insert(str);
        }     
    }
    {
        MutexLockGuard lock(mutex_);
        cache[channel_name].swap(update);//refresh cache;
    }
        printf("update cache\n");

    return host;
}



// 消息接收回调函数
void CRedisSubscriber::subscribe_callback(redisAsyncContext *redis_context,       //使用静态函数的好处？？？？？？？？
    void *reply, void *privdata)
{
    printf("in the subscribe_callback \n");
    if (NULL == reply || NULL == privdata) {
        printf("subscribe_callback wrong\n");
        return ;
    }

    // 静态函数中，要使用类的成员变量，把当前的this指针传进来，用this指针间接访问
    CRedisSubscriber *self_this = reinterpret_cast<CRedisSubscriber *>(privdata);
    redisReply *redis_reply = reinterpret_cast<redisReply *>(reply);

    // 订阅接收到的消息是一个带三元素的数组
    if (redis_reply->type == REDIS_REPLY_ARRAY && 
    redis_reply->elements == 3 && redis_reply->element[2]->type == REDIS_REPLY_STRING)
    {
        string channel = redis_reply->element[1]->str;
        string message = redis_reply->element[2]->str;
        size_t pos = message.find_first_of(":");
        if(pos == string::npos) 
        {
            printf("wrong command from publisher\n");
            return;
        } 
        string request = message.substr(0,pos);
        printf("request:%s\n",request.c_str());
        message = message.substr(pos+1);
        if(request == "Register")
        {
            MutexLockGuard lock(mutex_);
            cache[channel].insert(message);
        }
        else if(request == "unRegister")
        {
            MutexLockGuard lock(mutex_);
            cache[channel].erase(message);
        }
        else if(request == "refresh")
        {
            self_this->inquiryandrefresh(channel);
        }
    }
}



vector<string> CRedisSubscriber::discovery(const std::string channel_name)
{
    vector<string> host;
    {
        MutexLockGuard lock(mutex_);
        if(cache.find(channel_name)==cache.end())
        {
            printf("service subscribe has not been complished\n");
            cache[channel_name] = std::set<std::string>();
            subscribe(channel_name);
        }
        else
        {
            printf("find in cache\n");
            host = vector<string>(cache[channel_name].begin(),cache[channel_name].end());
        }
    }
    if(host.empty())
    {
        printf("search in redis\n");
        host = inquiryandrefresh(channel_name);
    }
    return host;
}