#include"Statistics.h"
#include<vector>
#include<map>
#include"Leastunrepliedmethod.h"

int main()
{
    Status::startcount("host1","method2000");
    Status::startcount("host1","method2");
    Status::startcount("host1","method3");
    Status::startcount("host1","method4");

    Status::startcount("host2","method2");
    Status::startcount("host3","method2");
    Status::startcount("host3","method2");
    Status::startcount("host2","method2");
    std::vector<std::string> host = {"host1","host2","host3"};
    std::string ans = Leastunrepliedmethod::select("","method2",host,"");

    printf("%s\n",ans.c_str());

    while(true)
    {
        sleep(10);
    }
    return 0;

}