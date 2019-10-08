#include"net/Tcpconnection.h"
#include<iostream>
#include<errno.h>
#include<stdio.h>
#include"net/Channel.h"
#include"net/Eventloop.h"
#include"base/Util.h"


using namespace std;

Tcpconnection::Tcpconnection(Eventloop*loop,std::string name,int sockfd)
    :loop_(loop),
    state_(Connecting),
    name_(name),
    sock_fd(sockfd),
    channel_(new Channel(loop,sock_fd))
{
    channel_->setreadcallback(bind(&Tcpconnection::handleread,this));
    channel_->setclosecallback(bind(&Tcpconnection::handleclose,this));
    channel_->setwritecallback(bind(&Tcpconnection::handlewrite,this));
    channel_->seterrorcallback(bind(&Tcpconnection::handleerror,this));
    setSocketNodelay(sock_fd);
}

Tcpconnection::~Tcpconnection()
{
}

void Tcpconnection::send(std::string message)
{
    if(state_ == Connected)
    {
        if(loop_->isinloopthread())
        {
            sendinloop(message);
        }
        else
        {
            loop_->runinloop(bind(&Tcpconnection::sendinloop,this,std::move(message)));
        }
    }
}

void Tcpconnection::sendinloop(std::string &message)//FIXME:: 没有用const为了修改message
{
    loop_->assertinloopthread();
    if(outbuffer_.empty())
    {
        if(writen(sock_fd,message) < 0 )
        {
            perror("written");
        }
        else
        {
            printf("write1\n");
        }
        
    }
    if(!message.empty())
    {
        outbuffer_ += message;
        //message.clear();// FIXME
        channel_->enablewriting();
        loop_->updatechannel(channel_);
    }
}

void Tcpconnection::shutdown()
{
}

void Tcpconnection::shutdowninloop()
{
}

void Tcpconnection::forceclose()
{
    assert(state_ == Connected || state_ == Disconnecting);
    state_ = Disconnecting;
    loop_->queueinloop(std::bind(&Tcpconnection::forcecloseinloop,shared_from_this()));//WHY shared_from_this?? why queue not runinloop?
}

void Tcpconnection::forcecloseinloop()
{
    loop_->assertinloopthread();
    assert(state_ == Connected || state_ == Disconnecting);
    handleclose();
}


void Tcpconnection::connectEstablished()
{
    loop_ ->assertinloopthread();
    assert(state_ == Connecting);
    state_ = Connected;
    channel_->set_events(EPOLLIN|EPOLLET);
    loop_->addchannel(channel_);
    //printf("addchanneltoloop + call connectioncallback\n");
    connectioncallback_(shared_from_this());
}

void Tcpconnection::connectDestroyed()
{
    assert(state_ == Connected||state_ == Disconnecting);
    channel_->set_events(0);
    loop_->updatechannel(channel_);
    //connectioncallback_(shared_from_this());
    loop_->removechannel(channel_);
    close(sock_fd);  //???
}


void Tcpconnection::handleread()
{
    bool zero = false;
    int read_num = readn(sock_fd,inbuffer_,zero);
    if(read_num < 0)
    {
        perror("read");
        handleerror();
        return;
    }
    else if(read_num > 0)
    {
        messagecallback_(shared_from_this(),inbuffer_);//messagecallback拿到信息后要清空输入缓冲区
    }
    
    if(zero)
        handleclose();

}

void Tcpconnection::handlewrite()
{
    printf("write2\n");
    if(writen(sock_fd,outbuffer_) < 0 )
    {
        perror("writen");
    }

    /* if(outbuffer_.size() > 0)
    {
        channel_->enablewriting();
        loop_->updatechannel(channel_);
    }*/
    if(outbuffer_.empty() && state_ == Disconnecting)
    {
        shutdowninloop();
    }
}

void Tcpconnection::handleclose()
{
    assert(state_ == Connected||state_ == Disconnecting);
    channel_->set_events(0);
    loop_->updatechannel(channel_);
    closecallback_(shared_from_this());
}

void Tcpconnection::handleerror()
{
}



