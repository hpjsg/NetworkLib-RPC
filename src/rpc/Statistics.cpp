#include"rpc/Statistics.h"

std::map<std::string,Status*> Status::PROVIDER_STATUS_MAP;
std::map<std::string,Status::mapwithlock> Status::METHOD_STATUS_MAP;
MutexLock Status::PROVIDER_MUTEX;
MutexLock Status::METHOD_MUTEX;

Status* Status::getstatus(std::string host)
{
    Status* init = new Status();
    {
        MutexLockGuard lock(PROVIDER_MUTEX);
        auto it = PROVIDER_STATUS_MAP.find(host);
        if(it == PROVIDER_STATUS_MAP.end())
        {
            PROVIDER_STATUS_MAP[host] = init;
        }
        else init = it->second;
    }
    return init;
}

Status::Status()
{
    unreplied.store(0);
    success.store(0);
    failed.store(0);
}

bool Status::removestatus(std::string host)
{
    {
        MutexLockGuard lock(PROVIDER_MUTEX);
        auto it = PROVIDER_STATUS_MAP.find(host);
        if(it != PROVIDER_STATUS_MAP.end())
        {
            PROVIDER_STATUS_MAP.erase(it);
            return true;
        }
    }
    return false;
}

Status* Status::getstatus(std::string host,std::string methodname)
{
    mapwithlock methodmap;
    {
        MutexLockGuard lock(METHOD_MUTEX);
        auto it = METHOD_STATUS_MAP.find(host);
        if(it == METHOD_STATUS_MAP.end())
        {
            METHOD_STATUS_MAP[host] = methodmap;
        }
        else methodmap = it->second;
    }

    Status* initstate = new Status();
    {
        MutexLockGuard lock(*(methodmap.mutex));
        auto it = methodmap.method_map->find(methodname);
        if(it == methodmap.method_map->end())
        {
            (*methodmap.method_map)[methodname] = initstate;
        }
        else initstate = it->second;
    }
    return initstate;
}


bool Status::removestatus(std::string host,std::string methodname)
{
    mapwithlock methodmap;
    {
        MutexLockGuard lock(METHOD_MUTEX);
        auto it = METHOD_STATUS_MAP.find(host);
        if(it != METHOD_STATUS_MAP.end())
        {
            methodmap = it->second;
        }
        else return false;
    }
    
    {
        MutexLockGuard(*methodmap.mutex);
        auto it = methodmap.method_map->find(methodname);
        if(it != methodmap.method_map->end())
        {
            methodmap.method_map->erase(it);
            return true;
        }
    }
    return false;
}

void Status::startcount(std::string host,std::string methodname)
{
    startcount(host,methodname,INT32_MAX-100);
}
void Status::startcount(std::string host, std::string methodname,int32_t threshold)
{
    Status* hoststatus = getstatus(host);
    Status* methodstatus = getstatus(host,methodname);
    if(methodstatus->unreplied.fetch_add(1) > threshold)
    {
        methodstatus->unreplied.fetch_sub(1);
    }
    else hoststatus->unreplied.fetch_add(1);

}
void Status::endcount(std::string host,std::string methodname,bool succeed)
{
    endcount(getstatus(host),succeed);
    endcount(getstatus(host,methodname),succeed);
}

void Status::endcount(Status* status,bool succeed)
{
    status->unreplied.fetch_sub(1);
    if(succeed)
    {
        status->success.fetch_add(1);
    }
    else
    {
        status->failed.fetch_add(1);
    }
}
