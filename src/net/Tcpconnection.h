#ifndef NET_TCPCONNECTION_H
#define NET_TCPCONNECTION_H

#include<string>
#include<functional>
#include<unistd.h>
#include<boost/noncopyable.hpp>
#include<boost/any.hpp>
#include<memory>

class Channel;
class Eventloop;


class Tcpconnection: boost::noncopyable,
                    public std::enable_shared_from_this<Tcpconnection>

{
    public:
        typedef std::shared_ptr<Tcpconnection> TcpconnectionPtr;
        typedef std::function<void(const TcpconnectionPtr&)> Connectioncallback;
        typedef std::function<void(const TcpconnectionPtr&)> Closecallback;
        typedef std::function<void(const TcpconnectionPtr&,std::string&)> Messagecallback;
        
        Tcpconnection(Eventloop* loop,std::string name,int sockfd);
        ~Tcpconnection();

        Eventloop* getloop() const{return loop_;}
        bool connected()const {return state_ == Connected;}
        void send(std::string message);
        std::string name(){return name_;}

        void setConnectioncallback(const Connectioncallback& cb)
        {connectioncallback_ = std::move(cb);}

        void setMessagecallback(const Messagecallback& cb)
        {messagecallback_ = std::move(cb);}

        void setClosecallback(const Closecallback& cb)
        {closecallback_ = std::move(cb);}

        void set_storage(const boost::any& storage)
        {storage_ = storage;}

        void connectEstablished();
        void connectDestroyed();

        void forceclose();
        void forcecloseinloop(); // 为了调试，应该是private

    private:
        void handleread();
        void handlewrite();
        void handleclose();
        void handleerror();
        void sendinloop(std::string& message);
        void shutdown();
        void shutdowninloop();
        
        Eventloop* loop_;
        enum STATE{Connecting,Connected,Disconnecting,Disconnected};
        STATE state_;//FIXME atomic variable;
        std::string name_;
        int sock_fd;
        std::shared_ptr<Channel> channel_;
        std::string inbuffer_;
        std::string outbuffer_;
        //bool error_;
        boost::any storage_;

        Connectioncallback connectioncallback_;
        Messagecallback messagecallback_;
        Closecallback closecallback_;
};

#endif
