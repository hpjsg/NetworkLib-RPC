#ifndef NET_CLIENT_H
#define NET_CLIENT_H

#include"net/Tcpconnection.h"
#include"net/Channel.h"
#include"net/Eventloop.h"

class Client
{
    public:
        typedef std::shared_ptr<Tcpconnection> TcpconnectionPtr;
        typedef Tcpconnection::Messagecallback Messagecallback;
        typedef Tcpconnection::Connectioncallback Connectioncallback;
        Client(Eventloop* loop);
        Client(Eventloop* loop,std::string ip,uint16_t port);
        void setconnectioncallback(Connectioncallback cb)
        {connectioncallback_ = std::move(cb);}

        void setmessagecallback(Messagecallback cb)
        {messagecallback_ = std::move(cb);}

        void Connect();
        void connectserver();
        void newconnection(int sockfd);
        void removeconnection(const TcpconnectionPtr&conn);

        void resetchannel();

        void start();
        void set_ip(std::string ip){ip_ = ip;}
        void set_port(uint16_t port){port_ = port;}


    private:
        Connectioncallback connectioncallback_;
        Messagecallback messagecallback_;
        Eventloop* loop_;
        TcpconnectionPtr connection_;
        MutexLock mutex_;
        std::string ip_;
        uint16_t port_;
        std::shared_ptr<Channel> connectchannel_;


};

#endif