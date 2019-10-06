#include"rpc/echo.pb.h"
#include"rpc/RpcClient.h"
#include"rpc/RpcServer.h"
#include"net/Eventloopthread.h"

using namespace rpc;
void* getchannel(void* arg)
{
    for(int i=1234;i<1500;i++)
    {
        Connpool* p = Connpool::getinstance();
        string host = uint16tostring(i) + "127.0.0.1";
        RpcChannelptr channel = p->getchannel(host);
        if(channel->isconnected()) printf("success\n");
    }
    return (void*)0;
}

int main(){

    Eventloopthread loopthread;      
    Eventloop* loop_ = loopthread.startloop();
    Connpool* p = Connpool::getinstance();
    p->setloop(loop_);             //为connectionpool　设置reactor loop

    int iret;
    pthread_t tids[10];
    for(int j=0;j<10;j++)
    {
        int iret = pthread_create(&tids[j],nullptr,&getchannel,nullptr);
		if(iret)
        {
			printf("create error,%s",strerror(iret));
			return iret;
		}
    }
    void *retval;
     	for(int i=0;i<4;i++){
		iret = pthread_join(tids[i],&retval);
		if(iret){
			printf("join error,%s",strerror(iret));
			return iret;
		}
		printf("%ld",(long)retval);
	}
	return 0;
}