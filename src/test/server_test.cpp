#include "net/Server.h"
#include "net/Eventloop.h"
#include <stdio.h>
#include"base/CurrentThread.h"
#include"net/Tcpconnection.h"

std::string message = "server say hello";

void onConnection(const TcpconnectionPtr& conn)
{
  if (conn->connected())
  {
    printf("onConnection(): tid=%d new connection [%s] \n",
           CurrentThread::tid(),
           conn->name().c_str());
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
         conn->send(message);
         buf.clear();
         //conn->forcecloseinloop();
}

int main(int argc, char* argv[])
{
  printf("main(): pid = %d\n", getpid());

  Eventloop loop;

  Server server(&loop,4,9981);
  server.setConnectionCallback(onConnection);
  server.setMessageCallback(onMessage);
  server.start();
  loop.loop();
}
