#include"rpc/gpbcodec.h"
#include<zlib.h>
#include<endian.h>
#include<string>
#include<vector>

using namespace rpc;

void gpbcodec::send(const TcpconnectionPtr&conn,const ::google::protobuf::Message& message)
{
    printf("gpbcodec send\n");
    std::string buf;
    fillbuffer(buf,message);
    conn->send(buf);
}

std::string int32tostring(int32_t  x)     //FIXME
{
    int32_t bex = htobe32(x);
    char* tmp = static_cast<char*>((void*)(&bex));
    return std::string(tmp,tmp + sizeof(int32_t));
}

void gpbcodec::fillbuffer(std::string& buf,const ::google::protobuf::Message& message)
{
    buf.clear();
    int lens = serializetobuffer(message,buf);
    int32_t Checksum = checksum(buf.c_str(),lens);
    buf = buf + int32tostring(Checksum);
    assert(buf.size()==static_cast<size_t>(lens + checksumlen));
    buf = int32tostring(buf.size()) + buf; 
}

bool gpbcodec::parsefrombuffer(std::string buf,google::protobuf::Message* message)
{
    return message->ParseFromString(buf);
}

int32_t gpbcodec::checksum(const void* buf,int len)
{
    return static_cast<int32_t>(adler32(1,static_cast<const Bytef*>(buf),len));
}

void gpbcodec::onmessage(const TcpconnectionPtr& conn, std::string& buf)
{
    while(buf.size() >= static_cast<uint32_t>(minmessagelen+headlen))
    {
        printf("gpbcodec receive message\n");
        const int32_t len = asInt32(buf.c_str());
        if(len < minmessagelen)
        {
            errorcallback_(conn,buf,invalidlength);
            break;
        }
        else if(buf.size() >= implicit_cast<size_t>(headlen+len))
        {
            Messageptr message(prototype_->New());
            errorcode err = parse(buf.substr(headlen,len),len,message.get());
            if(err == noerror)
            {
                messagecallback_(conn,message);
                buf = buf.substr(headlen+len); //FIXME
            }
            else
            {
                errorcallback_(conn,buf,err);
                break;
            }
        }
        else
        {
            break;// 半包
        }
    }
}

void gpbcodec::defaulterrorcallback(const TcpconnectionPtr& conn,std::string buf,errorcode errcode)
{
    if(conn && conn->connected())
    {
        conn->forceclose();
    }
}

int32_t gpbcodec::asInt32(const char* buf)
{
    int32_t be32 = 0;
    memcpy(&be32,buf,sizeof(be32));
    return be32toh(be32);
}

bool gpbcodec::validchecksum(const char*buf,int len)
{
    int32_t expected = asInt32(buf + len - checksumlen);
    int32_t Checksum = checksum(buf,len - checksumlen);
    return Checksum == expected;
}

gpbcodec::errorcode gpbcodec::parse(const std::string& buf,int len,google::protobuf::Message* message)
{
    errorcode error = noerror;
    if(validchecksum(buf.c_str(),len))
    {
        int32_t datalen = len - checksumlen;
        if(parsefrombuffer(buf.substr(0,datalen),message))
        {
        }
        else
        {
            error = parseerror;
        }
    }
    else 
    {
        error = checksumerror;
    }
    return error;
}

int gpbcodec::serializetobuffer(const google::protobuf::Message& message,std::string&buf)
{
    int byte_size = message.ByteSize();
    message.SerializeToString(&buf);
    return byte_size;
}