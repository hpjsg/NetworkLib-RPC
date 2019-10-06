#ifndef NET_SERVER_H
#define NET_SERVER_H

#include<functional>
#include<memory>
#include<map>
#include"net/Eventloop.h"
#include"net/Channel.h"
#include"net/Eventloopthreadpool.h"
#include"net/Tcpconnection.h"

typedef std::shared_ptr<Tcpconnection> TcpconnectionPtr;
typedef std::function<void(const TcpconnectionPtr&)>Connectioncallback;
typedef std::function<void(const TcpconnectionPtr&)>Closecallback;
typedef std::function<void(const TcpconnectionPtr&,std::string&)> Messagecallback;

class Server
{
    public:
        Server(Eventloop* loop,int threadnum,uint16_t port);
        ~Server(){}
        
        void start();
        void setMessageCallback(const Messagecallback& cb)
        {messagecallback_ = std::move(cb);}
        
        void setConnectionCallback(const Connectioncallback &cb)
        {connectioncallback_ = std::move(cb);}
        
        Eventloop* getloop() const{return loop_;}



    private:
        typedef std::map<std::string,TcpconnectionPtr>ConnectionMap;
        
        void newconnection();

        void removeconnection(const TcpconnectionPtr& conn);

        void removeconnectioninloop(const TcpconnectionPtr & conn);

        Eventloop* loop_;
        int threadnum_;
        std::unique_ptr<Eventloopthreadpool>eventloopthreadpool_;
        bool started_;
        uint16_t port_;
        int listenfd_;
        std::shared_ptr<Channel> acceptchannel_;

        int nextconnid_;
        Messagecallback messagecallback_;
        Connectioncallback connectioncallback_;
        ConnectionMap connections_;
        static const int MAXFDS = 100000;
};

#endif