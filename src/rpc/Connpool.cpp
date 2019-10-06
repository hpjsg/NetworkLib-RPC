#include"rpc/Connpool.h"

pthread_once_t Connpool::ponce_ = PTHREAD_ONCE_INIT;
Connpool* Connpool::connpool_ = nullptr;

namespace rpc
{
    uint16_t asUInt16(const char* buf)
    {
        uint16_t be16 = 0;
        memcpy(&be16,buf,sizeof(be16));
        return be16;
    }
}


Connpool::Connpool()
    :mutex_()
{
    connectionpool_ = new std::map<std::string,ConnectorPtr>();
    /*Eventloopthread loopthread;
    loop_ = shared_ptr<Eventloop>(loopthread.startloop());*/
    printf("Connpool ctor\n");
}

void Connpool::init()
{
    connpool_ = new Connpool();
}

RpcChannelptr Connpool::getchannel(std::string host)
{
    ConnectorPtr create;
    {
        MutexLockGuard lock(mutex_);
        auto it = connectionpool_->find(host);
        if(it != connectionpool_->end())
        {
            create = it->second;
            if(create->isdead())
            {
                create->retry();
            }
        }
        else
        {
            uint16_t port = asUInt16(host.c_str());
            string ip = host.substr(sizeof(uint16_t));
            printf("%u,%s\n",(unsigned int)port,ip.c_str());
            create = make_shared<Connector>(loop_,ip,port);
            //printf("creation\n");
            connectionpool_->insert({host,create});
        }
    }
    //printf("leave the mutex area\n");
    return create->get();                                                    //有可能多个线程到这里
}

