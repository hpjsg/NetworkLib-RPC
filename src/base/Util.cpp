#include"base/Util.h"
#include<unistd.h>
#include<fcntl.h>
#include<signal.h>
#include<errno.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netinet/tcp.h>

const int MAX_BUFF = 4096;
const int listen_num = 2048;

ssize_t readn(int fd,std::string &inbuffer,bool &zero)
{
    ssize_t nread = 0;
    ssize_t readsum = 0;
    while(true)
    {
        char buff[MAX_BUFF];
        if((nread = read(fd,buff,MAX_BUFF)) < 0)
        {
            if(errno == EINTR)
                continue;
            else if(errno == EAGAIN)
            {
                return readsum;
            }
            else
            {
                perror("read error");
                return -1;
            }
        }
        else if(nread == 0)
        {
            zero = true;
            break;
        }
        readsum += nread;
        inbuffer += std::string(buff,nread);
    }
    return readsum;
}

ssize_t writen(int fd, std::string &outbuffer)
{
    size_t nleft = outbuffer.size();
    ssize_t nwritten = 0;
    ssize_t writesum = 0;
    const char* ptr = outbuffer.c_str();
    while(nleft > 0)
    {
        if((nwritten = write(fd,ptr,nleft)) < 0)
        {
            if(nwritten < 0)
            {
                if(errno == EINTR)
                {
                    nwritten = 0;
                    continue;
                }
                else if(errno == EAGAIN)
                    break;
                else 
                    return -1;
            }
        }
        writesum += nwritten;
        nleft -= nwritten;
        ptr += nwritten;
    }
    if(nleft == 0)
        outbuffer.clear();
    else
        outbuffer = outbuffer.substr(writesum);

    return writesum;
} 

void setSocketNodelay(int fd)
{
    int enable = 1;
    setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,(void*)&enable,sizeof(enable));
}

int setSocketNonBlocking(int fd)
{
    int flag = fcntl(fd,F_GETFL,0);
    if(flag == -1)
        return -1;

    flag |= O_NONBLOCK;
    if(fcntl(fd,F_SETFL,flag) == -1)
        return -1;
    return 0; 
}

void shutDownWR(int fd)
{
    shutdown(fd,SHUT_WR);
}

int socket_bind_and_listen(int port)
{
    if(port<0||port>65536)
        return -1;
    int listen_fd = 0;
    if((listen_fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
        return -1;
    
    int optval = 1;
    if(setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(optval)) == -1)
        return -1;
    struct sockaddr_in server_addr;
    bzero((char*)&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons((unsigned short)port);

    if(bind(listen_fd,(struct sockaddr*)&server_addr,sizeof(server_addr)) == -1)
        return -1;

    if(listen_fd == -1)
    {
        close(listen_fd);
        return -1;
    }

    if(listen(listen_fd,listen_num) == -1)
        return -1;

    return listen_fd;
}