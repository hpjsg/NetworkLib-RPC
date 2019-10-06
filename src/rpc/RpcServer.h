#ifndef RPC_RPCSERVER_H
#define RPC_RPCSERVER_H

#include <string>
#include<google/protobuf/service.h>
#include"net/Server.h"
#include"rpc/RpcChannel.h"


using namespace rpc;
std::string uint16tostring(uint16_t x);

class RpcServer
{
    public:
        RpcServer(Eventloop* loop,int numthreads,uint16_t port);
        void registerservice(google::protobuf::Service*);
        void start();

    private:
        std::string address_;//port and ip
        void onconnection(const TcpconnectionPtr& conn);
        Server server_;
        std::map<std::string, google::protobuf::Service*> services_;
};

#endif
