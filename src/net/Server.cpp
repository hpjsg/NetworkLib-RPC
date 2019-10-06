#include"net/Server.h"
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include"base/Util.h"

Server::Server(Eventloop* loop,int threadnum,uint16_t port)
    :loop_(loop),
    threadnum_(threadnum),
    eventloopthreadpool_(new Eventloopthreadpool(loop_,threadnum_)),
    started_(false),
    port_(port),
    listenfd_(socket_bind_and_listen(port_)),
    acceptchannel_(new Channel(loop_,listenfd_)),
    nextconnid_(0)
{
    //SIGPIPE
    if(setSocketNonBlocking(listenfd_) < 0)
    {
        perror("set nonblock error");
        abort();
    }
}


void Server::start()
{
    if(!started_)
    {
        eventloopthreadpool_->start();
        acceptchannel_->set_events(EPOLLIN|EPOLLET);
        acceptchannel_->setreadcallback(bind(&Server::newconnection,this));
        loop_->addchannel(acceptchannel_);
        started_ = true;
    }
}

void Server::newconnection()
{
    struct sockaddr_in client_addr;
    memset(&client_addr,0,sizeof(struct sockaddr_in));
    socklen_t client_addr_len = sizeof(client_addr);
    int accept_fd = 0;
    while((accept_fd = accept(listenfd_,(struct sockaddr*)&client_addr,&client_addr_len)) > 0 )
    {
        if(accept_fd >= MAXFDS)         //限制并发链接数
        {
            close(accept_fd);
            continue;
        }
        if(setSocketNonBlocking(accept_fd) < 0)
        {
            return;
        }
        setSocketNodelay(accept_fd);

        char buf[32];
        snprintf(buf,sizeof(buf),"#%d",nextconnid_);
        ++nextconnid_;
        std::string connname = buf;
        setSocketNodelay(accept_fd);
        Eventloop* ioloop = eventloopthreadpool_->getnextloop();
        TcpconnectionPtr conn = make_shared<Tcpconnection>(ioloop,connname,accept_fd);  //不使用new by 陈硕??????
        connections_[connname] = conn;
        conn->setConnectioncallback(connectioncallback_);
        conn->setMessagecallback(messagecallback_); //Rpc messagecallback 由connectioncallback 绑定
        conn->setClosecallback(std::bind(&Server::removeconnection, this, std::placeholders::_1));//Fixme::unsafe

        ioloop->runinloop(std::bind(&Tcpconnection::connectEstablished,conn));
    }

}

void Server::removeconnection(const TcpconnectionPtr& conn)
{
    loop_->runinloop(std::bind(&Server::removeconnectioninloop,this,conn));
}

void Server::removeconnectioninloop(const TcpconnectionPtr& conn)
{
    loop_->assertinloopthread();
    size_t n = connections_.erase(conn->name());
    assert( n == 1);
    Eventloop* ioloop = conn->getloop();
    ioloop->queueinloop(std::bind(&Tcpconnection::connectDestroyed,conn));
}


