#include"Consistent_hash.h"
#include"Hashfunc.h"
#include<vector>
#include<string>

int main()
{
    std::vector<std::string>host = {"124.6.6.3","144.6.4.8","133.32.3.54","255.3.45.66"};
    for(int i=0;i<50;i++)
    {
        std::string ans = ConsistentHashmethod::select("abc","def",host,std::to_string(i+i*i));
        printf("%s\n",ans.c_str());
    }
    std::vector<std::string>server{"124.6.6.3","144.6.4.8","133.32.3.54"};
    for(int i=0;i<50;i++)
    {
        std::string ans = ConsistentHashmethod::select("abc","def",server,std::to_string(i+i*i));
        printf("%s\n",ans.c_str());
    }
}