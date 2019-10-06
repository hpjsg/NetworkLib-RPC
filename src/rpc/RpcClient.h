#ifndef RPC_RPCCLIENT_H
#define RPC_RPCCLIENT_H

#include<stdio.h>
#include<unistd.h>
#include<memory>
#include"net/Client.h"
#include"rpc/RpcChannel.h"
#include"rpc/future.h"
#include"rpc/Connpool.h"
#include"rpc/redis_subscriber.h"
#include"rpc/Leastunrepliedmethod.h"


using namespace rpc;

template<typename REQUEST,typename RESPONSE,typename STUB>
class RpcClient
{
    public:
        typedef shared_ptr<RESPONSE> ResponsePtr;
        typedef shared_ptr<future<ResponsePtr>>futureptr;
        RpcClient()
        {
            connection_ = Connpool::getinstance();
            register_center_ = CRedisSubscriber::getinstance();
            printf("rpcclient_ctor\n");
        }

        ResponsePtr call(REQUEST request,int i)
        {
            auto res = async_call(request,i);
            return res->get();
        }

        futureptr async_call(REQUEST request,int i)
        {   

            futureptr future_res = make_shared<future<ResponsePtr>>();

            const google::protobuf::MethodDescriptor* method = STUB::descriptor()->method(i);
            std::string method_name = method->name();
            std::string service_name = method->service()->full_name();
            const std::vector<std::string> host = register_center_->discovery(service_name);
            if(host.empty())
            {
                printf("no available server\n");
                return future_res;
            }

            std::string server = Leastunrepliedmethod::select(service_name,method_name,host);

            RpcChannelptr channel = connection_->getchannel(server);
            ResponsePtr response = make_shared<RESPONSE>();
            printf("ready to send\n");
            channel->CallMethod(method,NULL, &request, response.get(),NewCallback(this,&RpcClient::fill,future_res,response));
            return future_res;
        }
    private:
        void fill(futureptr f,ResponsePtr resp)
        {
            printf("fill the future");
            f->push(resp);
        }

        Connpool* connection_;
        CRedisSubscriber* register_center_;

        //RESPONSE response_;
};

#endif


