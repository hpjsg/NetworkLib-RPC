#include"rpc/Monitor.h"
#include"net/Eventloopthread.h"
int main()
{
  Eventloopthread loopthread;      
  Eventloop* loop_ = loopthread.startloop();
  Monitor::set_ip("127.0.0.1");
  Monitor::set_port(9981);
  Monitor* monitor = Monitor::getinstance();
  monitor->set_clean_period(30.0);
  monitor->set_validate_period(10.0);
  monitor->set_loop(loop_);
  monitor->start();
  while(true)
  {
    sleep(10);
  }
}
