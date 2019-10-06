#include "net/Eventloop.h"
#include "base/Thread.h"
#include "base/CurrentThread.h"
#include <stdio.h>

void threadFunc()
{
  printf("threadFunc(): pid = %d, tid = %d\n",
         getpid(),CurrentThread::tid());
}

int main()
{
  printf("main(): pid = %d, tid = %d\n",
         getpid(), CurrentThread::tid());
  Thread thread(threadFunc);
  thread.start();
  Eventloop loop;
  loop.loop();
  pthread_exit(NULL);
}
