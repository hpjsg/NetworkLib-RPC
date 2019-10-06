#ifndef RPC_RPCCHANNEL_H
#define RPC_RPCCHANNEL_H

#include<map>
#include<atomic>
#include<google/protobuf/service.h>
#include<google/protobuf/descriptor.h>
#include"base/Mutex.h"
#include"base/Condition.h"
#include"rpc/gpbcodecT.h"


namespace rpc{
class RpcChannel :public google::protobuf::RpcChannel
{
    public:
        RpcChannel(const TcpconnectionPtr& conn);
        RpcChannel();

        ~RpcChannel()override;

        void setConnection(const TcpconnectionPtr &conn)
        {
            conn_ = conn;
        }

        bool isconnected()
        {
            return conn_->connected();
        }
        void setservices(const std::map<std::string,google::protobuf::Service*>*services)
        {
            services_ = services;
        }

        void CallMethod(const ::google::protobuf::MethodDescriptor* method,
                        ::google::protobuf::RpcController* controller,
                        const  ::google::protobuf::Message* request,
                        ::google::protobuf::Message* response,
                        ::google::protobuf::Closure* done) override;
        
        void onmessage(const TcpconnectionPtr&conn, std::string& buf);

        void sethost(const std::string host){host_= host;}
        std::string gethost(){return host_;}

    private:
        void onrpcmessage(const TcpconnectionPtr& conn,const RpcMessagePtr& messageptr);
        
        void donecallback(::google::protobuf::Message*response,int64_t id);

        struct column
        {
            google::protobuf::Message* response;
            google::protobuf::Closure* done;
            column& operator= (column& from)
            {
               response = from.response;
                done = from.done;
                return *this;
            }
        };
        
        RpcCodec codec_;
        TcpconnectionPtr conn_;
        std::atomic_int64_t id_;
        std::string host_;
        MutexLock mutex_;
        std::map<int64_t,column> table_ ;
        const std::map<std::string,google::protobuf::Service*>* services_;
};
    typedef std::shared_ptr<RpcChannel> RpcChannelptr;
}

#endif

