#include"rpc/RpcChannel.h"
#include"rpc/rpc.pb.h"
#include"rpc/Statistics.h"

using namespace rpc;

RpcChannel::RpcChannel(const TcpconnectionPtr& conn)
    :codec_(std::bind(&RpcChannel::onrpcmessage,this,_1,_2)),         //id 没有赋初值？？？？？？？？？？？？？？？？
    conn_(conn),
    id_(0),
    services_(nullptr)
    {
        printf("rpcchannel ctor\n");
    }

RpcChannel::RpcChannel()
    :codec_(std::bind(&RpcChannel::onrpcmessage,this,_1,_2)),
    id_(0),
    services_(nullptr)
    {
        //printf("rpcchannel ctor\n");
    }

RpcChannel::~RpcChannel()
{
    for(auto& registeration:table_)
    {
        delete registeration.second.response;
        delete registeration.second.done; 
    }
}

void RpcChannel::CallMethod(const ::google::protobuf::MethodDescriptor* method,
                        ::google::protobuf::RpcController* controller,
                        const  ::google::protobuf::Message* request,
                        ::google::protobuf::Message* response,
                        ::google::protobuf::Closure* done) 
{
    RpcMessage message;
    message.set_type(REQUEST);
    int64_t id = id_.fetch_add(1);
    message.set_id(id);
    message.set_service(method->service()->full_name());
    message.set_method(method->name());
    message.set_request(request->SerializeAsString());

    column c = {response,done};
    {
        MutexLockGuard lock(mutex_);
        table_[id] = c;
    }
    printf("rpcchannel callmethod\n");
    codec_.send(conn_,message);
    Status::startcount(host_,method->name());
}

void RpcChannel::onmessage(const TcpconnectionPtr& conn,std::string&buf)
{
    printf("rpcchannel on message\n");
    codec_.onmessage(conn,buf);
}

void RpcChannel::onrpcmessage(const TcpconnectionPtr& conn,const RpcMessagePtr&  messageptr)
{
    printf("rpcchannel rpcmessage\n");
    assert(conn == conn_);
    RpcMessage& message = *messageptr;
    if(message.type() == RESPONSE)
    {
        int64_t id = message.id();
        assert(message.has_response()||message.has_error());

        column col = {nullptr,nullptr};
        {
            MutexLockGuard lock(mutex_);
            auto it = table_.find(id);
            if(it != table_.end())
            {
                col = it->second;
                table_.erase(it);
            }
        }
        if(col.response)
        {
            if(message.has_response())
            {
                col.response->ParseFromString(message.response());
            } 
            if(col.done)
            {
                printf("set future\n");
                col.done->Run();
            }
        }
        if(message.has_error())
        {
            Status::endcount(host_,message.method(),false);
        }
        else
        {
            Status::endcount(host_,message.method(),true);            
        }      
    }
    else if(message.type() == REQUEST)
    {
        ErrorCode error = WRONG_PROTO;
        if(services_)
        {
            auto it = services_->find(message.service());
            if(it != services_->end())
            {
                google::protobuf::Service* service = it->second;
                const google::protobuf::ServiceDescriptor* desc = service->GetDescriptor();
                const google::protobuf::MethodDescriptor* method
                    = desc->FindMethodByName(message.method());
                if(method)
                {
                    std::unique_ptr<google::protobuf::Message> request(service->GetRequestPrototype(method).New());
                    if(request->ParseFromString(message.request()))
                    {
                        google::protobuf::Message*response = service->GetResponsePrototype(method).New();
                        int64_t id = message.id();
                        service->CallMethod(method,NULL,request.get(),response,NewCallback(this,&RpcChannel::donecallback,response,id));
                        error = NO_ERROR;
                        
                    }
                    else
                    {
                        error = INVALID_REQUEST;
                    }
                }
                else
                {
                    error = NO_METHOD;
                }
            }
            else
            {
                error = NO_SERVICE;
            }
            
        }
        else
        {
            error = NO_SERVICE;
        }

        if(error != NO_ERROR)
        {
            RpcMessage response;
            response.set_type(RESPONSE);
            response.set_id(message.id());
            response.set_error(error);
            codec_.send(conn_,response);
        }
    }
}

void RpcChannel::donecallback(::google::protobuf::Message*response,int64_t id)
{
    printf("send_back response\n");
    std::unique_ptr<google::protobuf::Message> tmp(response);
    RpcMessage message;
    message.set_type(RESPONSE);
    message.set_id(id);
    message.set_response(response->SerializeAsString());
    codec_.send(conn_,message);

}