#include"base/Thread.h"
#include"base/CurrentThread.h"
#include<stdio.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <linux/unistd.h>
#include<assert.h>
#include <stdint.h>


namespace CurrentThread
{
    __thread int t_cachedtid = 0;
    __thread const char* t_threadname = "default";
}

pid_t gettid()
{
    return static_cast<pid_t>(::syscall(SYS_gettid));
}

void CurrentThread::cachetid()
{
    if(t_cachedtid == 0)
    {
        t_cachedtid = gettid();
    }
}

struct ThreadData
{
    typedef Thread::ThreadFunc ThreadFunc;
    ThreadFunc func_;
    std::string name_;
    pid_t * tid_;
    CountDownLatch *latch_;

    ThreadData(const ThreadFunc &func,const std::string& name,pid_t *tid,CountDownLatch *latch)
        : func_(func),
        name_(name),
        tid_(tid),
        latch_(latch)
    {
    }
    void runinThread()
    {
        *tid_ = CurrentThread::tid();
        latch_->countdown();
        tid_ = nullptr;
        latch_ = nullptr;

        CurrentThread::t_threadname = name_.empty()?"Thread":name_.c_str();
        prctl(PR_SET_NAME,CurrentThread::t_threadname);
        printf("ready to call the functor\n");
        func_();
        CurrentThread::t_threadname = "finished";  
    }
};

void *startthread(void* obj)
{
    ThreadData* data = static_cast<ThreadData*>(obj);
    printf("start run the thread\n");
    data->runinThread();
    delete data;
    return NULL;
}

Thread::Thread(const ThreadFunc &func,const std::string &n)
    :started_(false),
    joined_(false),
    pthreadId_(0),
    tid_(0),
    functor_(func),
    name_(n),
    latch_(1)
{
    printf("thread ctor\n");
    //setdefaultname();
}

Thread::~Thread()
{
    if(started_&&!joined_)
        pthread_detach(pthreadId_);
}

void Thread::setdefaultname()
{
    if(name_.empty())
    {
        name_ = "Thread";
    }
}

void Thread::start()
{
    printf("start new thread\n");
    assert(!started_);
    started_ = true;
    ThreadData* data = new ThreadData(functor_,name_,&tid_,&latch_);
    if(pthread_create(&pthreadId_,NULL,startthread,data))
    {
        started_ = false;
        delete data;
    }
    else
    {
        latch_.wait();
        assert(tid_ > 0);
    }
}

int Thread::join()
{
    assert(started_);
    assert(!joined_);
    joined_ = true;
    return pthread_join(pthreadId_,NULL);
}
