#include "net/Channel.h"
#include "net/Eventloop.h"

#include <stdio.h>
#include <sys/timerfd.h>
#include<memory>
#include<strings.h>

Eventloop* g_loop;

void timeout()
{
  printf("Timeout!\n");
  g_loop->quit();
}

int main()
{
  Eventloop loop;
  g_loop = &loop;

  int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  shared_ptr<Channel> chan(new Channel(&loop, timerfd));
  chan->setreadcallback(timeout);
  chan->set_events(EPOLLIN | EPOLLET);
  g_loop->addchannel(chan);


  struct itimerspec howlong;
  bzero(&howlong, sizeof howlong);
  howlong.it_value.tv_sec = 5;
  ::timerfd_settime(timerfd, 0, &howlong, NULL);

  loop.loop();

  close(timerfd);
}
