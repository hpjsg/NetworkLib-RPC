#include"rpc/Monitor.h"
#include<string.h>
#include"net/Timestamp.h"

pthread_once_t Monitor::ponce_ = PTHREAD_ONCE_INIT;
Monitor* Monitor::monitor_ = nullptr;
int Monitor::port_ = 6237;
std::string Monitor::ip_ = "127.0.0.1"; 
redisContext * Monitor::context_ = nullptr;

void Monitor::init()
{
    monitor_ = new Monitor();
    context_ = redisConnect(ip_.c_str(), port_);
    if (context_ == NULL || context_->err)
    {
        if (context_) 
        {
            printf("Error: %s\n", context_->errstr);
            // handle error
        } 
        else
        {
            printf("Can't allocate redis context\n");
        }
    }
}

Monitor::Monitor()
{
    validate_period_ = clean_period_ = 0;
    loop_ = nullptr;
}

void Monitor::start()
{
    if(validate_period_ ==0 ||clean_period_ ==0||loop_ == nullptr)
    {
        printf("please initialize ip port and loop first\n");
        return;
    }
    loop_->runevery(clean_period_,&Monitor::clean);
}

void Monitor::clean()
{
    int cursor = 0;
    redisReply* reply = reinterpret_cast<redisReply*>
                            (redisCommand(context_,"SCAN %d",cursor));
    if(reply->type != REDIS_REPLY_ARRAY)
    {
        printf("scan error\n");
        return ;
    }
    redisReply* head = reply->element[0]; //cursor for next call

    if(head->type == REDIS_REPLY_STRING)
    {
        do
        {
            redisReply* body = reply->element[1];
            int num = body->elements;
            for(int i=0;i<num;i++)
            {
                redisReply* item = body->element[i];
                if(item->type == REDIS_REPLY_STRING)
                {
                    string message(item->str);
                    size_t pos = message.find(":");
                    string channel = message.substr(pos+1);
                    printf("%s\n",channel.c_str());
                    zremrangebyscore(channel);//remrange here
                }
            }
            reply =  reinterpret_cast<redisReply*>
                            (redisCommand(context_,"SCAN %s",head->str));
            head = reply->element[0];
        }while(strcmp(head->str,"0") != 0);

        redisReply* body = reply->element[1];
        int num = body->elements;
        for(int i=0;i<num;i++)
        {
            redisReply* item = body->element[i];
            if(item->type == REDIS_REPLY_STRING)
            {
                string message(item->str);
                size_t pos = message.find(":");
                string channel = message.substr(pos+1);
                zremrangebyscore(channel);//remrange here
            }
        }

    }
}

void Monitor::zremrangebyscore(std::string channel)
{
    Timestamp deadline = substractime(Timestamp::now(),monitor_->validate_period_);
    redisReply* reply = reinterpret_cast<redisReply*>
                            (redisCommand(context_,"ZREMRANGEBYSCORE %s -inf (%ld",channel.c_str(),deadline));
    if(reply->type == REDIS_REPLY_INTEGER && reply->integer != 0)
    {
        redisCommand(context_,"PUBLISH %s %s",channel.c_str(),"refresh:");
        printf("clean out\n");
    }   
}