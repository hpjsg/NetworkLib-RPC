#ifndef RPC_REDISCONNECTIONPOOL_H
#define RPC_REDISCONNECTIONPOOL_H



#include <hiredis/async.h>
#include "base/Blockingqueue.h"
#include "net/Eventloop.h"
#include "rpc/hiredis_adapter.h"

class RedisConnectionpool
{
    public:
        RedisConnectionpool(int connection_num);

        void init(Eventloop* loop,std::string ip, uint16_t port);

        redisAsyncContext * getasynccontext()
            {return async_connection_pool.take();}

        redisContext * getsynccontext()
            {return sync_connection_pool.take();} 

        void returnasynccontext(redisAsyncContext* conn)
            {async_connection_pool.put(conn);}

        void returnsynccontext(redisContext* conn) 
            {sync_connection_pool.put(conn);}
        
    private:
        int connection_num_;

        Blockingqueue<redisAsyncContext *> async_connection_pool; 

        Blockingqueue<redisContext *> sync_connection_pool;

    private:
        static void connect_callback(const redisAsyncContext *redis_context,
            int status);

        // 断开连接的回调
        static void disconnect_callback(const redisAsyncContext *redis_context,
            int status);
};

#endif