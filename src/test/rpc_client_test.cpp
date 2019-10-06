#include"rpc/echo.pb.h"
#include"rpc/RpcClient.h"
#include"net/Eventloopthread.h"

using namespace rpc;
int main()
{   

    Eventloopthread loopthread;      // 不能定义成局部变量 loop得是一直活着的 考虑有办法的话还是放在程序里对用户不可见
    Eventloop* loop_ = loopthread.startloop();
    Connpool* p = Connpool::getinstance();
    p->setloop(loop_);
    CRedisSubscriber* subscribe = CRedisSubscriber::getinstance();
    subscribe->setloop(loop_);
    subscribe->init();
    RpcClient<echo::EchoRequest,echo::EchoResponse,echo::EchoService_Stub> rpcclient;
    echo::EchoRequest request;
    request.set_payload("1111000");
    auto reply = rpcclient.async_call(request,0);
    reply->inspect();
    return 0;
}