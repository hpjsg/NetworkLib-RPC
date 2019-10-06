#ifndef RPC_LEASTUNREPLIEDMETHOD_H
#define RPC_LEASTUNREPLIEDMETHOD_H

#include<vector>
#include <ctime>
#include"rpc/Statistics.h"

#define random(x) (rand()%(x))
class Leastunrepliedmethod
{
    public:
        static std::string select(std::string service,std::string method,std::vector<std::string>providers,std::string arg="")
        {
            int length = providers.size();
            int leastunreplied = -1;
            int leastcount = 0;
            int leastindex[length];

            for(int i=0; i<length;i++)
            {
                std::string host = providers[i];
                int32_t unreplied = Status::getstatus(host,method)->getunreplied();
                if(leastunreplied == -1||unreplied < leastunreplied)
                {
                    leastunreplied = unreplied;
                    leastcount = 1;
                    leastindex[0] = i;
                }
                else if(unreplied == leastunreplied)
                {
                    leastindex[leastcount++] = i;
                }
            }

            if(leastcount == 1)
            {
                return providers[leastindex[0]];
            }

            srand((int)time(0));
            return providers[leastindex[random(leastcount)]];
        }
};

#endif