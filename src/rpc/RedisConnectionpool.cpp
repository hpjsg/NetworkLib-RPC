#include"rpc/RedisConnectionpool.h"

RedisConnectionpool::RedisConnectionpool(int connection_num)
            :connection_num_(connection_num),
            async_connection_pool(connection_num),
            sync_connection_pool(connection_num)
{
}

void RedisConnectionpool::init(Eventloop* loop,std::string ip,uint16_t port)
{
    for(int i = 0; i < connection_num_; i++)
    {
        redisAsyncContext *_redis_context = redisAsyncConnect(ip.c_str(), port);  
        if (NULL == _redis_context)
        {
            printf(": Connect redis failed.\n");
        }

        if (_redis_context->err)
        {
            printf(": Connect redis error: %d, %s\n", 
                _redis_context->err, _redis_context->errstr);    
        }

        int ret = redisnetattach(_redis_context, loop);
        assert(ret == 0);   

        _redis_context->data = this;
        redisAsyncSetConnectCallback(_redis_context, 
            RedisConnectionpool::connect_callback);

        redisAsyncSetDisconnectCallback(_redis_context,
            RedisConnectionpool::disconnect_callback);

        redisContext *context_ = redisConnect(ip.c_str(), port);
        if (context_ == NULL || context_->err)
        {
            if (context_) 
            {
                printf("Error: %s\n", context_->errstr);
                // handle error
            } 
            else
            {
                printf("Can't allocate redis context\n");
            }
        }
        else
        {
            printf("redis connect success\n");
            sync_connection_pool.put(context_);
        }
    }
}



void RedisConnectionpool::connect_callback(const redisAsyncContext *redis_context,
    int status)
{
    RedisConnectionpool *self_this = reinterpret_cast<RedisConnectionpool*>(redis_context->data);
    if (status != REDIS_OK)
    {
        printf(": Error: %s\n", redis_context->errstr);
    }
    else
    {
        printf(": Redis connected!");
        self_this->async_connection_pool.put(const_cast<redisAsyncContext*>(redis_context));   ///////这里是只读指针，但是模板要求指针常量
    }
}

void RedisConnectionpool::disconnect_callback(
    const redisAsyncContext *redis_context, int status)
{
    if (status != REDIS_OK)
    {
        // 这里异常退出，可以尝试重连
        printf(": Error: %s\n", redis_context->errstr);
    }
}
