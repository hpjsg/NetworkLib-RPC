#include "rpc/redis_publisher.h"
#include"net/Eventloopthread.h"
int main(int argc, char *argv[])
{

    Eventloopthread loopthread;      
    Eventloop* loop_ = loopthread.startloop();
    CRedisPublisher* publisher = CRedisPublisher::getinstance();
    CRedisPublisher::setloop(loop_);
 
 
    bool ret = publisher->init();
    if (!ret)
    {
        printf("connect failed.");
        return 0;
    }
 

    publisher->registration("test-channel","first message");
    publisher->registration("test-channel","third message");
    publisher->registration("test-channel","second message");
    publisher->registration("test-channel","fourth message");
    sleep(5);
    publisher->unregistration("test-channel","first message");
    while(true)
    {
        sleep(10);
    }
 
    return 0;
}
