#ifndef RPC_HIREDIS_ADAPTER_H
#define RPC_HIREDIS_ADAPTER_H

#include<hiredis/hiredis.h>
#include<hiredis/async.h>
#include"net/Eventloop.h"

struct redisnetevents
{
    redisAsyncContext * context_;
    shared_ptr<Channel> channel_;
    Eventloop* loop_;
    void readcallback()
    {
        redisAsyncHandleRead(context_);
    }
    void writecallback()
    {
        redisAsyncHandleWrite(context_);
    }
};


static void redisaddread(void* privdata)
{
    redisnetevents *e = (redisnetevents*)privdata;
    e->channel_->enablereading();
    e->loop_->updatechannel(e->channel_);
}

static void redisdelread(void* privdata)
{
    redisnetevents *e = (redisnetevents*)privdata;
    e->channel_->disablereading();
    e->loop_->updatechannel(e->channel_);
}

static void redisaddwrite(void* privdata)
{
    redisnetevents *e = (redisnetevents*)privdata;
    e->channel_->enablewriting();
    e->loop_->updatechannel(e->channel_);
}

static void redisdelwrite(void* privdata)
{
    redisnetevents *e = (redisnetevents*)privdata;
    e->channel_->disablewriting();
    e->loop_->updatechannel(e->channel_);
}
static void rediscleanup(void* privdata)
{
    redisnetevents*e = (redisnetevents*)privdata;
    if(!e)
    {
        return;
    }
    e->channel_->set_events(0);
    e->loop_->updatechannel(e->channel_);
    e->loop_->removechannel(e->channel_);

    //FIXME
}


static int redisnetattach(redisAsyncContext* ac,Eventloop*loop)
{
    redisContext * c = &(ac->c);
    redisnetevents* e;
    if (ac->ev.data != NULL)
        return REDIS_ERR;
    e = (redisnetevents*)calloc(1, sizeof(*e));
    e->context_ = ac;

    ac->ev.addRead = redisaddread;
    ac->ev.delRead = redisdelread;
    ac->ev.addWrite = redisaddwrite;
    ac->ev.delWrite = redisdelwrite;
    ac->ev.cleanup = rediscleanup;
    ac->ev.data = e;
    e->channel_ = make_shared<Channel>(loop,c->fd);
    e->channel_->set_events(EPOLLIN|EPOLLET); 
    e->loop_ = loop;
    e->loop_->addchannel(e->channel_);
    e->channel_->setreadcallback(std::bind(&redisnetevents::readcallback,e));
    e->channel_->setwritecallback(std::bind(&redisnetevents::writecallback,e));
    return REDIS_OK;
}

#endif