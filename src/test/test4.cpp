
#include "rpc/rpc.pb.h"
#include "rpc/gpbcodecT.h"
#include <stdio.h>

using namespace rpc;

void rpcMessageCallback(const TcpconnectionPtr&,const RpcMessagePtr&)
{
}

Messageptr g_msgptr;
void messageCallback(const TcpconnectionPtr&,const Messageptr& msg)
{
  g_msgptr = msg;
}

void print(const std::string& buf)
{
  printf("encoded to %zd bytes\n", buf.size());
  for (size_t i = 0; i < buf.size(); ++i)
  {
    unsigned char ch = static_cast<unsigned char>(buf[i]);

    printf("%2zd:  0x%02x  %c\n", i, ch, isgraph(ch) ? ch : ' ');
  }
}

int main()
{
  RpcMessage message;
  message.set_type(REQUEST);
  message.set_id(2);
  std::string buf1, buf2;
  RpcCodec codec(rpcMessageCallback);
  codec.fillbuffer(buf1, message);
  RpcMessage receive;
  codec.onmessage(TcpconnectionPtr(),buf1);
}