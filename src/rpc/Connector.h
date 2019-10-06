#ifndef RPC_CONNECTOR_H
#define RPC_CONNECTOR_H


#include"base/Condition.h"
#include"base/Mutex.h"
#include"net/Client.h"
#include"rpc/RpcChannel.h"



namespace rpc{
    class Connector
    {
        public:
            Connector(Eventloop* loop,std::string ip,uint16_t port);

            RpcChannelptr get();

            void set(const TcpconnectionPtr& conn);

            bool isdead();

            void retry();

        private:

            volatile bool set_;
            Client client_;
            MutexLock mutex_;
            Condition cond_;
            RpcChannelptr channel_;

    };
}



#endif