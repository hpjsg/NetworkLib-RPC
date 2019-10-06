#ifndef RPC_CONSISTENT_HASH_H
#define RPC_CONSISTENT_HASH_H

#include<map>
#include<vector>
#include"base/Mutex.h"

class MD5hashfunc;

class Hashfunc
{
    public:
        virtual uint32_t Hash(std::string) = 0;
};


class ConsistentHash
{
    public:
        ConsistentHash(Hashfunc* func,int vnum,const std::vector<std::string>* host);
        ~ConsistentHash();

        std::string Getserver(std::string key);

        void insertnode(std::string key);
        void deletenode(std::string key);
        const std::vector<std::string>* gethost()
        {return host_;}

    private:
        Hashfunc* func_;
        int vnode_num;
        std::map<uint32_t,std::string> nodes_;
        const std::vector<std::string>* host_;

};


class ConsistentHashmethod
{
    public:
        static std::map<std::string,ConsistentHash*> selectors_;
        static std::string select(std::string service,std::string method,const std::vector<std::string>&providers,std::string arg);
    private:
        static MutexLock mutex_;
        static  MD5hashfunc func_;
};

#endif