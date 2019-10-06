#ifndef RPC_CONNPOOL_H
#define RPC_CONNPOOL_H

#include<boost/noncopyable.hpp>
#include<map>
#include"net/Eventloop.h"
#include"net/Eventloopthread.h"
#include"rpc/Connector.h"
using namespace rpc;                             //host = 端口+ip

typedef shared_ptr<Connector> ConnectorPtr;
class Connpool: boost:: noncopyable
{
    public:
        static Connpool* getinstance()
        {
            pthread_once(&ponce_,&Connpool::init);
            //printf("enter the connection pool\n");
            return connpool_;
        }

        RpcChannelptr getchannel(std::string host);
        
        inline void setloop(Eventloop* loop)
        {
            loop_ = loop;
        }
    private:
        Connpool();
        ~Connpool();

        static void init();

        static pthread_once_t ponce_;
        static Connpool* connpool_;
        std::map<std::string, ConnectorPtr>* connectionpool_;
        MutexLock mutex_;
        Eventloop* loop_; /// 这个怎么处理？得保证他的生命期最长，同时也是唯一的

};

#endif
