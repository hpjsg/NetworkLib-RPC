#include "rpc/redis_subscriber.h"
#include "net/Eventloopthread.h"
 
int main(int argc, char *argv[])
{
    Eventloopthread loopthread;      // 不能定义成局部变量 loop得是一直活着的 考虑有办法的话还是放在程序里对用户不可见
    Eventloop* loop_ = loopthread.startloop();
    CRedisSubscriber* subscriber = CRedisSubscriber::getinstance() ;
    CRedisSubscriber::setloop(loop_);

    bool ret = subscriber->init();
    if (!ret)
    {
        printf("Init failed.\n");
        return 0;
    }

    subscriber->subscribe("test-channel");
    sleep(2);
    vector<string>res = subscriber->discovery("test-channel");
    for(auto &x:res)
    {
        printf("%s\n",x.c_str());
    }

    while (true)
    {
        sleep(10);
    }
 
    return 0;
}
