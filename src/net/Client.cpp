#include"net/Client.h"
#include<errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include"rpc/RpcChannel.h"
#include"base/Util.h"

using namespace rpc;

Client::Client(Eventloop* loop)
    :loop_(loop)
{
    printf("client ctor\n");
}

Client::Client(Eventloop* loop,std::string ip,uint16_t port)
    :loop_(loop),
    ip_(ip),
    port_(port)
{
    //printf("client ctor\n");
}
//Client::~Client()
//{
//}



void Client::start()
{
    loop_->runinloop(std::bind(&Client::Connect,this));
    //printf("client start\n");
}

void Client::Connect()
{  // printf("client Connect\n");
    int  sockfd;
    struct sockaddr_in  servaddr;
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(setSocketNonBlocking(sockfd) < 0)
    {
        return;
    }
    setSocketNodelay(sockfd);
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port_);
    inet_pton(AF_INET,ip_.c_str(),&servaddr.sin_addr);
    int ret = connect(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr));
    int savederrno = (ret==0)? 0:errno;
    if(savederrno == 0||savederrno == EINPROGRESS||savederrno==EISCONN||savederrno==EINTR)
    {
        connectchannel_.reset(new Channel(loop_,sockfd));
        connectchannel_->set_events(EPOLLOUT|EPOLLET);
        connectchannel_->setwritecallback(std::bind(&Client::connectserver,this));
        loop_->addchannel(connectchannel_);
    }
}

void Client::connectserver()
{   
    //printf("connectserver\n");
    connectchannel_->set_events(0);
    loop_->updatechannel(connectchannel_);
    loop_->removechannel(connectchannel_);
    int sockfd = connectchannel_->fd();
    loop_->queueinloop(std::bind(&Client::resetchannel,this));//FIXME
    newconnection(sockfd);
}

void Client::resetchannel()
{
    connectchannel_.reset();
}
void Client::newconnection(int sockfd)
{
    loop_->assertinloopthread();
    TcpconnectionPtr conn = make_shared<Tcpconnection>(loop_,"",sockfd);
    conn->setConnectioncallback(connectioncallback_);
    conn->setMessagecallback(messagecallback_);
    conn->setClosecallback(std::bind(&Client::removeconnection,this,_1));
    {
        MutexLockGuard lock(mutex_);     // 不用锁，判断connection==nullptr，在reset前不赋值阻塞应该同样效果 //client 持有conn的意义何在？？？
        connection_ = conn;
    }
    //printf("newconnection\n");
    conn->connectEstablished();
}

void Client::removeconnection(const TcpconnectionPtr &conn)
{
    {
        MutexLockGuard lock(mutex_);
        assert(connection_ == conn);
        connection_.reset();
    }
    loop_->queueinloop(std::bind(&Tcpconnection::connectDestroyed,conn));
}
