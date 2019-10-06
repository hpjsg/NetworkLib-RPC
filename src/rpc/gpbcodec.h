#ifndef RPC_GPBCODEC_H
#define RPC_GPBCODEC_H

#include<memory>
#include<type_traits>
#include<google/protobuf/message.h>
#include<boost/noncopyable.hpp>
#include<functional>
#include"net/Tcpconnection.h"

namespace rpc
{
    typedef std::shared_ptr<google::protobuf::Message> Messageptr;  
    typedef std::shared_ptr<Tcpconnection> TcpconnectionPtr;

    template<typename To, typename From>
    inline To implicit_cast(From const &f)
    {
        return f;
    }

    template<typename To, typename From>
    inline ::std::shared_ptr<To> down_pointer_cast(const ::std::shared_ptr<From>& f)
    {
        #ifndef NDEBUG
        assert(f == NULL || dynamic_cast<To*>(f.get())!= NULL);
        #endif
        return ::std::static_pointer_cast<To>(f);
    }
    class RpcMessage;
    typedef std::shared_ptr<RpcMessage> RpcMessagePtr;

    class gpbcodec:boost::noncopyable
    {
        public:
            const static int headlen = sizeof(int32_t);
            const static int checksumlen = sizeof(int32_t);

            enum errorcode
            {
                noerror = 0,
                invalidlength,
                checksumerror,
                invalidnamelen,
                //unknownmessagetype,
                parseerror
            };
            
            
            typedef std::function<bool(const TcpconnectionPtr&,std::string)>Rawmessagecallback;

            typedef std::function<void(const TcpconnectionPtr&,const Messageptr&)> Gpbmessagecallback;

            typedef std::function<void(const TcpconnectionPtr&,std::string,errorcode)>Errorcallback;

            gpbcodec(const:: google::protobuf::Message* prototype,
                    const Gpbmessagecallback& messagecb,
                    const Errorcallback&errorcb = defaulterrorcallback)
                    :prototype_(prototype),
                    messagecallback_(std::move(messagecb)),
                    errorcallback_(std::move(errorcb)),
                    minmessagelen(checksumlen)
                    {
                    }

            virtual ~gpbcodec() = default;

            void send(const TcpconnectionPtr& conn, const ::google::protobuf::Message& message);
            
            void onmessage(const TcpconnectionPtr& conn,std::string& buf);

            virtual bool parsefrombuffer(std::string buf,google::protobuf::Message* message);
            virtual int serializetobuffer(const google::protobuf::Message&messsage,std::string& buf);

            errorcode parse(const std::string& buf,int len,google::protobuf::Message* message);
            void fillbuffer(std::string&buf,const google::protobuf::Message& message);

            static int32_t checksum(const void* buf,int len);
            static bool validchecksum(const char*buf,int len);
            static int32_t asInt32(const char*buf);
            static void defaulterrorcallback(const TcpconnectionPtr&,std::string,errorcode);
        private:
            const google::protobuf::Message* prototype_;
            Gpbmessagecallback messagecallback_;
            Errorcallback errorcallback_;
            const int minmessagelen;
    };


}

#endif