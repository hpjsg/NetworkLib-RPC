#ifndef RPC_GPBCODECT_H
#define RPC_GPBCODECT_H

#include"rpc/gpbcodec.h"
//using namespace rpc;
namespace rpc
{
    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_3;
    template<typename MSG,typename CODEC=gpbcodec>
    class gpbcodecT
    {
    public:
        typedef std::shared_ptr<MSG> MSGPtr;
        typedef std::function<void(const TcpconnectionPtr&,const MSGPtr&)> gpbmessagecallback;
        typedef gpbcodec::Errorcallback Errorcallback;

        explicit gpbcodecT(const gpbmessagecallback& messagecb,const Errorcallback& errorcb = gpbcodec::defaulterrorcallback)
            :messagecallback_(messagecb),
            codec_(&MSG::default_instance(),
            std::bind(&gpbcodecT::onrpcmessage,this,_1,_2),
            errorcb)
        {
        }
        
        void send(const TcpconnectionPtr&conn,const MSG&message)
        {
            printf("gpbcodecT send\n");
            codec_.send(conn,message);
        }
        
        void onmessage(const TcpconnectionPtr& conn,std::string& buf)
        {
            printf("gpbcodecT on message\n");
            codec_.onmessage(conn,buf);
        }
        void onrpcmessage(const TcpconnectionPtr& conn,const Messageptr&message)
        {
            printf("gpbcodecT rpcmessage\n");
            messagecallback_(conn,down_pointer_cast<MSG>(message));
        }

        void fillbuffer(std::string&buf,const MSG& message)
        {
            codec_.fillbuffer(buf,message);
        }

    private:
        gpbmessagecallback messagecallback_;
        CODEC codec_;
    };
    typedef gpbcodecT<RpcMessage> RpcCodec;
}

#endif