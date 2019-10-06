#ifndef BASE_UTIL_H
#define BASE_UTIL_H

#include<cstdlib>
#include<string>

ssize_t readn(int fd,std::string &inbuffer,bool &zero);
ssize_t writen(int fd,std::string &outbuffer);
int setSocketNonBlocking(int fd);
void setSocketNodelay(int fd);
int socket_bind_and_listen(int port);

#endif
