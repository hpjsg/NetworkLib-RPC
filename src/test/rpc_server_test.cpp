#include"rpc/echo.pb.h"
#include"rpc/RpcServer.h"
#include<unistd.h>
#include"rpc/redis_publisher.h"

namespace echo
{

class EchoServiceImpl : public EchoService
{
 public:
  virtual void Echo(::google::protobuf::RpcController* controller,
                    const ::echo::EchoRequest* request,
                    ::echo::EchoResponse* response,
                    ::google::protobuf::Closure* done)
  {
    //LOG_INFO << "EchoServiceImpl::Solve";
    response->set_payload(request->payload());
    printf("%s\n",request->payload().c_str());
    done->Run();
  }
};

}  // namespace echo

int main(int argc, char* argv[])
{
  Eventloopthread loopthread;   
  Eventloop* loop_ = loopthread.startloop();
  CRedisPublisher* publisher = CRedisPublisher::getinstance();
  publisher->setloop(loop_);
  publisher->init();
  int nThreads = 2;
  uint16_t port = 7749;
  echo::EchoServiceImpl impl;
  Eventloop loop;
  RpcServer server(&loop,nThreads,port);
  server.registerservice(&impl);
  server.start();
  loop.loop();
}
