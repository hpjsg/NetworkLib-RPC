#ifndef RPC_STATISTICS_H
#define RPC_STATISTICS_H

#include<atomic>
#include <map>
#include"base/Mutex.h"

class Status
{
    public:
        Status();

        static Status* getstatus(std::string host);
        static bool removestatus(std::string host);

        static Status* getstatus(std::string host,std::string methodname);
        static bool removestatus(std::string host,std::string methodname);

        static void startcount(std::string host,std::string methodname);
        static void startcount(std::string host, std::string methodname,int32_t threshold);

        static void endcount(std::string host,std::string methodname,bool succeed);
        static void endcount(Status* status,bool succeed);

        int getunreplied(){return unreplied.load();}
 
        struct mapwithlock
        {
            MutexLock* mutex;
            std::map<std::string,Status*>* method_map;
            mapwithlock()
            {
                mutex = new MutexLock();
                method_map = new std::map<std::string,Status*>();
            }

            mapwithlock operator=(mapwithlock& rhs)
            {
                method_map = rhs.method_map;
                mutex = rhs.mutex;
                return *this;
            }
        };
        
        void print_unreplied()
        {
            printf("%d\n",unreplied.load());
        };

        void print_failed()
        {
            printf("%d\n",failed.load());
        }

        void print_success()
        {
            printf("%ld\n",success.load());
        }
  
    private:
        //bool operator==(const Status&);
        std::atomic<int32_t> unreplied;
        std::atomic<int64_t> success;
        std::atomic<int32_t> failed;
        static std::map<std::string,Status*>PROVIDER_STATUS_MAP;
        static std::map<std::string,mapwithlock> METHOD_STATUS_MAP;
        static MutexLock PROVIDER_MUTEX;
        static MutexLock METHOD_MUTEX;
};

#endif