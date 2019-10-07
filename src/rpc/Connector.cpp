#include "rpc/Connector.h"

using namespace rpc;
Connector::Connector(Eventloop* loop,std::string ip,uint16_t port)
    :set_(false),
    client_(loop,ip,port),
    mutex_(),
    cond_(mutex_),
    channel_(new RpcChannel())
{  
    //printf("Connector ctor\n");
    channel_->sethost(ip); 
    client_.setconnectioncallback(std::bind(&Connector::set,this,_1));
    client_.setmessagecallback(std::bind(&RpcChannel::onmessage,channel_.get(),_1,_2));
    client_.start();
}

RpcChannelptr Connector::get()
{
    MutexLockGuard lock(mutex_);
    //printf("in connector get\n");
    while(set_ == false)
    {
        cond_.wait();
    }
    return channel_;
}
void Connector::set(const TcpconnectionPtr& conn)
{
    MutexLockGuard lock(mutex_);
    //printf("in connector set\n");
    if(conn->connected())
    {
        channel_->setConnection(conn);
        set_ = true;
        cond_.notifyall();
    }
    else
    {
        client_.start();
    }
}
bool Connector::isdead()
{
    MutexLockGuard lock(mutex_);
    if(set_ == true && !channel_->isconnected())
        return true;
    else 
        return false;
}
void Connector::retry()
{
    MutexLockGuard lock(mutex_);
    set_ = false;
    client_.start();
}