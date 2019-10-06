#ifndef RPC_MONITOR_H
#define RPC_MONITOR_H

#include<boost/noncopyable.hpp>
#include <hiredis/hiredis.h>
#include"net/Eventloop.h"

class Monitor: boost::noncopyable
{
    public:
         static Monitor* getinstance()
        {
            pthread_once(&ponce_,&Monitor::init);
            //printf("enter the connection pool\n");
            return monitor_;
        }
        
        void start();
        void stop();

        void set_clean_period(double val){clean_period_ = val;}
        void set_validate_period(double val){validate_period_ = val;}
        void set_loop(Eventloop* loop){loop_ = loop;}

        static void set_ip(string ip){ip_ = ip;}
        static void set_port(int port){port_ = port;}
        
    private:
        Monitor();
        ~Monitor();

        static void init();
        static void clean();
        static void zremrangebyscore(string channel);


        static pthread_once_t ponce_;
        static Monitor* monitor_;
        static string ip_;
        static int port_;
        double validate_period_;
        double clean_period_;
        static redisContext *context_;
        Eventloop* loop_;
};

#endif