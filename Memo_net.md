## net
net中比较关键的几个类的设计如下：

### channel
对fd进行了包装，结构如下：
* fd可读或可写时或错误对应的回调函数
* 对fd感兴趣的events和epoll返回的revents
* handleevent函数根据revents调用对应的回调函数
* handleevent执行中有可能会调用回调函数使得channel析构，因此设置inhandling标志，在析构函数中检查标志，防止对象析构。

### EPOLL
对epoll进行了包装，结构如下：
* fd2channel数组，根据epoll返回fd找到对应channel对象。没有使用map，而是使用了数组下标为fd存储对应channel对象，查找效率更高。
* 提供了增加、修改、删除channel的对应接口
* poll函数在while循环中，阻塞在epoll_wait上，直到有可读写的文件描述符。
* getactivechannel 根据可读写的fd找到对应channel，填写revents字段，并返回活跃的channel对象列表。


### Eventloop
reactor的主循环
* eventfd用于异步唤醒loop(只关心EPOLLIN事件) 
* timerqueue(set)用于管理定时器 
* pendingfunctors_是任务队列
* loop函数不可跨线程使用(assert断言检查当前是否在对象构造的线程中)。在while循环中先调用epoll获取activechannel,然后handleevent执行相应回调函数，接着执行pendingfunctors_中由其他线程送入的任务。
* runinloop函数，将任务functor在loop构造的线程中执行。如果当前线程不是loop线程，functor放入pendingfunctors_中，并往eventfd中写入数据，唤醒阻塞在epoll上的loop线程。
* 通过timerqueue_的add和concel接口来实现定时任务的添加和取消


### Eventloopthread
将Eventloop封装在一个thread中
* Thread对象创建线程，在新线程中创建loop填入loop_字段，并执行loop.loop();
* startloop函数接口启动Thread，在loop_填入后返回loop_

### Eventloopthreadpool
初始化时创建numthreads_个Eventloopthread，按照Round Robin依次取用，接口getnextloop

### Tcpconnection
对channel进一步的封装
* Tcpconnection 在Server中的connectionmap中以Tcpconnectionptr对象管理。它继承了enable_shared_from_this类，在它的成员函数中用到this指针的地方可以传递一个shared_from_this(),一个指向this的shared_ptr并且与Server中的Tcpconnectionptr共享管理权。
* connectioncallback_ 在将channel对应的fd加入epoll后即连接建立起来后调用
* messagecallback_ 在read到东西后调用
* inbuffer和outbuffer作为输入输出缓冲区，因为Tcp连接的缓冲区是有限的，防止信息较长导致读写阻塞用缓冲区来缓存。
* handleread,handlewrite,handleclose分别注册到channel对应的callback中。由于使用epoll边沿触发，要一直读写到EAGIN错误才可返回。read返回0，说明对端主动close（见Uitl的readn和writen）。handleread根据读的结果调用messagecallback或handleerror或在readn读到0后调用handleclose。
* send调用sendinloop来保证多线程下调用send输出缓冲区安全。如果输出缓冲区为空就尝试直接写fd，如果信息没有发送完就缓存到输出缓冲区的尾部，在channel的events中加入EPOLLOUT
* handleclose 将channel的events设为0(掩码为0表示不关心读写)，调用closecallback在对象析构前将存在server的connectionmap中的Tcpconnectionptr对象删掉。closecallback绑定server的removeconnection。
* connectiondestroyed是connection析构前最后调用的一个函数，在closecallback中被调用，将channel从epoll中去掉，将对应fd关闭。


### Server
* 一个主loop,accept请求。主loop上添加acceptchannel用于监听某个端口
* 一个Eventloopthreadpool 
* acceptchannel的readcallback为Server::newconnection。当acceptchannel可读时(EPOLLET要一直读到accept返回错误),从线程池中取一个loop和accept_fd一起构造成Tcpconnecionptr,绑定对应回调函数。
* connectionmap持有Tcpconnectionptr，key为每个Tcpconnection的name

### Timer && Timestamp
Timer是对定时任务的封装，主要成员有：
* 超时的时间戳及到时后调用的回调函数
* 是否重复标志和重复的定时间隔。
Timestamp用int64标记时间，精确到微秒 用gettimeofday获取当前时间(返回struct timeval有 .tv_sec .tv_usec两个成员)。

### Timerqueue
用于管理定时任务。
* 用红黑树Timers来管理多个Timer，之所以不用<Timestamp,Timer*>的map是防止有多个同时到时的时钟，可以使用muiltimap。但是使用set存放pair<Timestamp,Timer*>更为方便。
* timerfd对应最早到时的任务，timerchannel封装了timerfd，只关心EPOLLIN事件，readcallback为对Timers的处理函数Timerqueue::handleread。
* handleread获取当前时间戳，读Timerfd，调用getexpired获得当前超时的Timer的列表，并将他们从Timers中删除。然后分别调用Timer对应的回调函数。最后调用reset函数检查超时列表中的Timer是否有repeat重复标志，如果有重新插入到Timers中，并用Timers中最早到时的时间戳更新timerfd。
* 这里有个问题是，timer在chenshuo老师那里使用的是裸指针，在他的书里写了可以使用unique_ptr来管理。问题是unque_ptr不允许使用等号赋值。在使用STL的容器时复制时需要使用移动语义。可以使用make_move_iterator来底层调用std::move()(生成一个不具名的右值引用来调用移动构造函数或移动赋值函数)。这里有个问题是，由于我使用了set，set返回的是const_iterator表示它指向对象是const不可修改的，而vector对象是非const的。在getexpired函数中，需要将set的一部分放入vector(数据是pair<int,unique_ptr>)。如果数据不是unique_ptr的话可以vector.insert(vector.iteratorpos,set.begin(),set.end())，insert会把const对象转为非const来放入vector（具体怎么做的？）但考虑unique_ptr这里使用了make_move_iterator底层就会变成 pair = std::move(const pair) 就无法调用移动构造函数，而是调用pair（cosnt pair&） 这个delete function。const pair = move(const pair) 或pair = move(pair);都可以调用移动语义。最后我使用了shared_ptr，代价可能高了一些，也可以不用set改用muiltimap，因为map只有key是const的，value是非const不会有上述问题。



