#include"rpc/Consistent_hash.h"
#include<string>
#include"rpc/Hashfunc.h"

std::map<std::string,ConsistentHash*> ConsistentHashmethod::selectors_;
MutexLock ConsistentHashmethod::mutex_;
MD5hashfunc ConsistentHashmethod::func_;

ConsistentHash::ConsistentHash(Hashfunc* func,int vnum,const std::vector<std::string>* host)
    :func_(func),
    vnode_num(vnum),
    host_(host)
{
    for(auto &ip:*host_)
    {
        for(int i=0;i<vnode_num;i++)
        {
            std::string id = ip + std::to_string(i);
            uint32_t hashval = func_->Hash(id);
            nodes_.insert({hashval,ip});
        } 
    }
}

ConsistentHash::~ConsistentHash()
{
}

std::string ConsistentHash::Getserver(std::string key)
{
    uint32_t hashval = func_->Hash(key);
    auto it = nodes_.lower_bound(hashval);
    if(it == nodes_.end())
    {
        return nodes_.begin()->second;
    }
    return it->second;
}

void ConsistentHash::insertnode(std::string key)
{
    for(int i=0;i<vnode_num;i++)
    {
        std::string id = key + std::to_string(i);
        uint32_t hashval = func_->Hash(id);
        nodes_.insert({hashval,key});
    }
}

void ConsistentHash::deletenode(std::string key)
{
    for(int i=0;i<vnode_num;i++)
    {
        std::string id = key + std::to_string(i);
        uint32_t hashval = func_->Hash(id);
        auto it = nodes_.find(hashval);
        if(it!=nodes_.end())
        {
            nodes_.erase(it);
        }
    }
}

std::string ConsistentHashmethod::select(std::string service,std::string method,const std::vector<std::string>& providers,std::string arg)
{
    if(arg.empty())
    {
        printf("error arg\n");
    }
    std::string key = service + method;
    ConsistentHash* selector = nullptr;
    {
        MutexLockGuard lock(mutex_);
        auto it = selectors_.find(key);
        if(it != selectors_.end())
        {
            selector = it->second;
        }
    }
    if(selector == nullptr||selector->gethost() != &providers)
    {
        selector = new ConsistentHash(&func_,100,&providers);
        {
            MutexLockGuard lock(mutex_);
            selectors_[key] = selector;
        }
    }
    return selector->Getserver(arg);
}

