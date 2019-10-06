#include"rpc/RpcServer.h"
#include"rpc/redis_publisher.h"

using namespace rpc;
std::string uint16tostring(uint16_t x)     //FIXME
{
    char* tmp = static_cast<char*>((void*)(&x));
    return std::string(tmp,tmp + sizeof(uint16_t));
}

RpcServer::RpcServer(Eventloop* loop,int numthread,uint16_t port)
    :server_(loop,numthread,port)
    {
        address_ = uint16tostring(port)+"127.0.0.1";
        server_.setConnectionCallback(std::bind(&RpcServer::onconnection,this,_1));
        printf("RpcServer ctor\n");
    }

void RpcServer::registerservice(google::protobuf::Service* service)
{
    const google::protobuf::ServiceDescriptor*desc = service->GetDescriptor();
    services_[desc->full_name()] = service;
    CRedisPublisher* publisher = CRedisPublisher::getinstance();
    bool ret = publisher->registration(desc->full_name(),address_);
    if(ret)
    {
          printf("Register service success\n");
    }
    else
    {
        printf("Register error\n");
    }
    
}

void RpcServer::start()
{
    server_.start();
    printf("rpcserver start\n");
}

void RpcServer::onconnection(const TcpconnectionPtr& conn)
{
    if(conn->connected())
    {
        RpcChannelptr channel = make_shared<RpcChannel>(conn);
        channel->setservices(&services_);
        printf("set service\n");
        conn->setMessagecallback(std::bind(&RpcChannel::onmessage,channel.get(),_1,_2));
        conn->set_storage(channel);
    }
    else
    {
        conn->set_storage(RpcChannelptr());
    }

}
