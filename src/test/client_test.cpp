#include <functional>
#include "net/Client.h"
#include "net/Eventloopthread.h"
#include "net/Tcpconnection.h"
#include"base/CurrentThread.h"

typedef Client::TcpconnectionPtr TcpconnectionPtr;
std::string message = "client say hello";

void onConnection(const TcpconnectionPtr& conn)
{
  if (conn->connected())
  {
    printf("onConnection(): tid=%d new connection [%s] \n",
           CurrentThread::tid(),
           conn->name().c_str());
    conn->send(message);
  }
  else
  {
    printf("onConnection(): tid=%d connection [%s] is down\n",
           CurrentThread::tid(),
           conn->name().c_str());
  }
}

void onMessage(const TcpconnectionPtr& conn,std::string& buf)
{
    printf("onMessage(): tid=%d received %zd bytes from connection [%s] \n",
         CurrentThread::tid(),
         buf.size(),
         conn->name().c_str());
    printf("%s\n",buf.c_str());
         //conn->forcecloseinloop();
}

int main()
{
    Eventloopthread loopthread;      
    Eventloop* loop = loopthread.startloop();
    Client client(loop,"127.0.0.1",9981);
    client.setconnectioncallback(onConnection);
    client.setmessagecallback(onMessage);
    client.start();
    while(true)
    {
      sleep(10);
    }
}