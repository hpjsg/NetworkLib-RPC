# rpc
rpc中比较主要的几个类的接口如下：
### gpbcodec
gpbcodec是一个基于google protobuf的编解码器,是rpcchannel的底层实现
* send 调用fillbuffer将google::protobuf::Message(派生类)对象序列化成string。在头部加入信息长度，在尾部加入校验和(使用adler32算法)，均用一个int_32表示。google::protobuf的序列化不包括信息长度和类型名，所以手动在头部加入长度，类型名可以有method name在.proto文件中查到接受message的类型。调用Tcpconnection的send将信息发出去。序列化(最初使用google protobuf Message的SerializeWithCachedSizesToArray接口，但后来发现这是个纯虚函数接口)使用Message的SerializeToString接口。
* onmessage 
需要解决半包和分包的问题。当buf中数据长度大于一个包的最小长度时开始读数据。根据最前面的四个字节得到头部的信息长度，当buf中的数据长度大于信息长度时说明收到了一个完整的包。调用Message*prototype ->new()得到一个相同类型的对象。然后调用parse函数开始解包。在parse中调用validchecksum检查包尾部的checksum。最后调用Message的ParseFromString接口反序列化信息放入刚刚用new()初始化的对象中。解包过程如果返回错误调用相应的错误回调函数，否则调用messagecallback_，并清除输入缓冲区中的相关信息。

### gpbcodecT
是gpbcodec引入了模板，参数为concrete_Message 也就是.proto文件中定义的具体的Message类。(继承了google protobuf Message，是在rpc传输中使用的message类（rpcmessage）而非用户定义的标示request和response的message类)
* send 和 onmessage 均调用gpbcodec的同名函数
* gpbcodec的messagecallback_绑定gpbcodecT的onrpcmessage，调用gpbcodecT的messagecallback_。这里用到了down_pointer_cast向下转换，将gpbcodec的messagecallback中的Messageptr转换成concrete_Messageptr(基类指针转派生类指针是向下)
最好可以加入安全检查，检查传入模板参数是否继承了Message类。这里我目前没有考虑。
参考了muduo，借鉴了google的implicit_cast和down_pointer_cast分别用于up_cast和 down_cast(在gpbcodec.h中)

* static_cast：这个是最常用的类型转换，凡是C++隐式执行的类型转换都可以用static_cast显式完成。在隐式转换时有时编译器会有警告信息，但是显示转换(使用static_cast)就不会有。static_cast还会进行一些基础的类型检查，能在编译期发现错误。
* const_cast：从名字可以看出和const有关，这个转换的作用是去除或添加const特性，它可以将一个const变量转换为非const变量，或将一个非const变量转换为const变量
* dynamic_cast：dynamic_cast依赖于RTTI信息，在转换时，dynamic_cast会检查转换的source对象是否真的可以转换成target类型，这种检查不是语法上的，而是真实情况的检查。被转换的类型必须是多态（即有虚函数）。dynamic_cast操作符，将基类类型的指针或引用安全地转换为派生类型的指针或引用。 
* interpret_cast：interpret意思为重新解释，意思为将数据的二进制格式重新解释，它依赖机器。

### rpc.proto
rpcmessage是用在底层rpc传输时的message结构。其中request/response是string类型用于存放序列化后的请求或者回复(用户定义的concrete_Message类的序列化)。type用来标示request or response,id为信息在rpcchannel对象中的id，service存放service_name,method存放method_name。使用google protobuf生成rpc.pb.h和rpc.pb.cpp文件。

### rpcchannel
单独持有一个Tcpconnection,是对Tcpconnection的进一步包装，加入了gpbcodec编码器
* map<int,column>table 用来保证消息id和repsonse&&回调函数的对应关系，id对于每个rpcchannel唯一，使用原子变量保证线程安全(一个rpcchannel对象持有唯一的Tcp连接，多线程调用时需要记住每个消息和其response对象及callback对应关系)。
* map<servicename,service*>* services_ 当rpc连接建立后，服务器端的rpcchannel对象的service_指针指向rpcserver的service的注册表。
* callmethod 是用来发送请求的函数。根根据concrete_Message结构的request构造要发送的RpcMessage结构,将key = id, val = {response*,callback}放入map table中， 调用gpbcodecT<rpcmessage>的send发送出去。
* onmessage 调用gpbcodecT的onmessage
* onrpcmessage 绑定到gpbcodecT的messagecallback_，在gpbcodecT::onrpcmessage中被调用。rpcmessage对象的type字段如果是RESPONSE，从map table中根据信息的id取出对应的response*填入，并调用相应回调函数（要记得拿出后删除掉map中的对应项）。如果type字段是REQUEST,根据rpcmessage的service字段在*services_注册表查询到对应google protobuf Service对象,找到对应Servicedescriptor。再结合rpcmessage的method字段得到到MethodDescriptor,然后获得response和request对应的concrete_message结构原型，调用Service对应的callmethod函数，函数参数的closure回调函数为rpcchannel::donecallback,用于将得到的response发送回客户端。如果调用方法过程中有错误，填写rpcmessage的error字段并发回客户端。

消息流动的方向是：
RpcChannel::callmethod ---> gpbcodecT::send --->gpbcodec::send 发送
RpcChannel::onmessage ---> gpbcodecT::onmessage--->gpbcodec::onmessage--->gpbcodec::messagecallback_--->gpbcodecT::onrpcmessage--->gpbcodecT::messagecallback_--->RpcChannel::onrpcmessage
gpbcodec上的消息类型是google::protobuf::Message，gpbcodecT上是用于rpc传输的concrete_Message Rpcmessage,RpcChannel上的信息类型是用户定义的concrete_Message type 也就是request和response的具体类型。

### rpcserver
是对server的进一步封装
* services_ 是服务注册表，key为service_name,val为google::protobuf::Service*。
* registerservice 为注册服务的接口。将service的相关条目放入rpcserver对象的service_表中。然后在Redis注册中心注册这个服务，调用CRedispublisher::registration来实现注册中心的注册功能。
* onconnection 绑定到server的connectioncallback_。根据Tcpconnection构造RcpChannel,设置RcpChannel的services_指针指向rpcserver的map services_注册表。
将rpcchannel的onmessage绑定到Tcpconnection的messagecallback_。将rpcchannel对应的shared_ptr存到Tcpconnection的storage_字段里。

### rpcclient
rpc的客户端的实现,是一个模板类。模板参数分别是用户定义的request和reponse的concrete_Message type和concrete_Service_stub服务代理类。
提供了async_call异步接口和call同步接口。
同步接口call通过调用async_call实现。
* async_call
返回一个future结构。调用future::get接口可以获得response。先定义response和future的prototype，然后根据Service_name去注册中心查询该service可用的服务器地址列表hostlist。接口为CRedissubscriber::discovery。然后根据Service_name，method_name以及available hostlist调用相关loadbalance算法进行客户端负载均衡。这里使用Leastunrepliedmethod::select接口，返回推荐的host。调用rpc连接池提供的接口connpool::getchannel得到连接到host的rpcchannelptr对象。调用rpcchannel的callmethod接口来发送请求。callmethod函数参数中的回调函数为rpcclient::fill函数，用于拿到response后将调用future::push将response填入对应的future结构的response字段。callmethod发送request后，async_call就会结束返回future结构。


### future
future用于实现async_call异步调用接口。
* get接口用于获取response。如果put标志是false，则阻塞在condition上，直到put标志变为true，返回response字段。
* set接口用于填入response。修改put标志为true,调用condition.notify通知等待线程。
* 条件变量要用mutex锁保护，put标志最好设置成volatile类型，每次读取都从内存中，而不是用cache,保证put标志的变化能被其他线程看到。

### connector
当连接池中没有某个host的rpc连接时，构造connector对象来建立这个连接。
* 在构造函数中调用client::start用于发起对host的连接。将connectioncallback绑定到connector::set，用于将已经建立的Tcpconnection绑定到新建的rpcchannel上，并condition.notify阻塞在get上的线程。
* get接口用于获取rpcchannelptr对象。isdead接口是防止有rpcchannel的Tcpconnection断掉了，这时候需要调用retry接口重连。

### connpool
Rpc连接池。利用pthread_once保证多线程下connpool对象只创建一次。
* map<string,connector>connectionpool 用于存放host对应的connector对象。
* getchannel接口 如果connectionpool中找到了host对应的connector对象：若是对象的Tcpconnection断掉了就connector::retry，否则返回connector::get。若是没有找到connector对象，就创建一个放入connectionpool中，返回connector::get。
这里主要的难点是对于一个新的host，只创建一次rpc连接。如果使用map<address,rpcchannel>的数据结构，那么锁的颗粒度会比较粗。addrA访问map(使用mutex保护)，发现对应rpcchannel不存在。这时需要建立一条新的rpcchannel。但是addrA在创建rpcchannel的过程中不能释放mutex_，不然其他线程可能再次访问map查找addrA的rpcchannel，会重复建立连接。但是这样导致其他的address也不能访问map。如果服务器相应时间比较长，map就会锁很长一段时间。一种解决方法是：使用map<address,mutex>为每一个address创建一把锁，拿到这把锁后释放保护map的锁去创建连接。其余线程在拿到保护address的锁后double check一下rpcchannel是否存在。我这里是使用了异步的创建连接方式————创建connector对象，connector对象的构造函数会将发起连接某个address的任务放到它的loop的任务队列中去执行，所以它可以很快返回。这时就可以释放保护map的锁给其他address访问。而相同address会找到同一个connector对象，调用它的get接口获得rpcchannel对象。

### hiredis_adapter
hiredis是redis官方提供的C语言客户端。它的异步调用的事件是注册到libevent事件库上的，它为此写了adapter文件。这里我使用我net作为事件注册和通知模块。写了hiredis_adapter.h，主要就是绑定redisAsyncContext的读写事件的注册和相应回调函数。(见redisnetattach)

### Redisconnectionpool
redis的hiredis客户端连接成功后返回redisContext/redisAsyncContext对象。但是这个对象并不是线程安全的，比如redisAsyncContext它把命令送入缓冲区中就返回了。但这个缓冲区是没有保护的，也就是说这个对象不能多线程同时使用。所以需要使用连接池，在每次发送消息给redis前取出连接，发送完后放回连接池。使用Boundedblocking作为连接池结构。
* init 异步连接调建立用redisAsyncConnect，将返回的redisAsyncContext与loop绑定（redisattach），redisAsyncContext->data可以存放参数供connect_callback调用。连接成功的connect_callback将redisAsyncContext放入blockingqueue中(使用了const_cast将只读指针转换成了普通指针)。disconnect_callback可以退出也可以重连。

### Blockingqueue
底层使用boost::circular_buffer。它是对vector的包装，是一个长度有限的vector。当放满元素后，push_front会用新元素覆盖队尾的元素，push_back会覆盖队首元素，可做FIFO和LIFO的cache。unread表示队列中元素的个数，这样取元素时只需要调整unread而省去了一次pop操作。如果元素是对象，pop还会调用对象的析构函数，有一定的时间开销。
* 主要接口为take和put，使用两个个condition和一个mutex来同步操作（条件变量要用锁保护！！）
* 由于Redisconnectionpool存放的是指针，所以模板应该对指针进行偏特化。类模板可以偏特化，类的成员函数和普通函数无法偏特化，只能重载。
必须偏特化的原因是指针的性质特殊，比如函数<T>function(const T)，参数是一个T类型常量。如果T对应char*,那么参数就不能为const char*这个常量指针（指向的量是常量）而应该是char* const。<const T*>function(const T*) 的T对应char，参数可以使用只读指针。


### CRedisPublisher
封装了server端用于和redis注册中心通信的操作。在静态变量定义时创建了唯一的对象。
注册中心的数据结构使用ZSET,每个host的score为注册的时间。ZSET会根据score排序。
* getinstance 返回唯一的实例
* registration 是服务注册接口,调用ZADD命令将{service，host}注册到redis上。在loop上添加一个定时任务，每隔一个周期调用refresh函数来刷新host的score(注册时间)，Timermap存放{service，host}对应的定时任务对象Timerptr。最后调用publish发布“service register：host”的消息给所有subscribe了service的客户端。
* refresh接口用于定期刷新redis上{service，host}的注册时间。
* publish接口用于发布消息。调用PUBLISH service，host命令发布消息给所有subscribe了service的客户端。
* unregistration 取消注册命令。删除loop上的{service，host}对应的定时任务。调用ZREM命令删除redis的{service，host}。
最后调用publish发布"service unregister:host"消息给所有subscribe了service的客户端。
发送命令给redis之前要从redis连接池Redisconnectionpool中取出连接(异步or同步)，发送完命令后再将连接放回连接池中。接口为Redisconnectionpool::getasynccontext和Redisconnectionpool::returnasynccontext。

### CRedisSubscriber
封装了client端用于和redis注册中心通信的操作。在静态变量定义时创建了唯一的对象。
* cache 为本地缓存的Service对应的可用hostlist。
* subscriber redis的subscribe命令和unsubscribe命令必须独占Tcp连接，使用了这两个命令的连接不能用来发送其他命令。因此，在客户端subscribe和unsubscribe从异步连接池中取连接发送命令，其他命令从同步连接池中取连接使用。
发送SUBSCRIBE service命令监听关于service的所有发布的消息。在subscribe_callback中，解析消息，根据是register/unregister/refresh操作，分别在本地cache中插入/删除{service，host}，以及调用inquiryandrefresh向redis重新获取关于service的所有host更新cache内容。
* inquiryandrefresh 向redis发送ZRANGE service 0 -1 命令获取service的所有host存放在一个局部set和一个vector里。vector用于返回查询结果，局部的set调用swap函数与cache[service]交换，更新cache同时自动释放旧的对象。
* discovery 是rpcclient根据Service查询可用hostlist的接口。先在本地cache中查询service，如果找不到对应项，就说明还没有subscribe这个service，先subscribe该service，然后调用inquiryandrefresh刷新缓存并返回结果。如果找到了，且可用hostlist不为空则直接返回，如果为空调用inquiryandrefresh接口。

### monitor
monitor是运行在注册中心上的，用于管理超时为注册的host。调用redis的SCAN来遍历所有的key值，然后按照score的范围调整member。每一次调用SCAN cursor都会返回两个element，第一个是一个string为下一次scan的cursor，第二个是本次搜索到的key的列表。最初cursor为0，要一直检索到cursor再次为0为止(这一次返回的key列表也要处理)
* clean 调用SCAN获取key值，然后调用zremrangebyscore来处理key中超时的host
* zremrangebyscore 调用ZREMRANGEBYSCORE去掉Service的hostlist中注册时间在range中的host。这个range为 [-inf,Timestamp::now-validate_period) validate_period是注册的有效时长。redis返回去掉的member个数，如果不为0，PUBLISH所有subscribe了Service的客户端"refresh"，让它们刷新cache。
clean绑定在loop上是一个定时任务，每clean_period清除一次过期host地址。

### Statistics
是为了统计发送到host调用{service，method}方法的请求，有多少没有收到回复unreplied，有多少次成功收到回复success，有多少次失败failed。在LB方法leastunreplied中用这些信息来做客户端的负载均衡。这些统计量使用原子变量来保证可以多线程安全。
* PROVIDER_STATUS_MAP map<string,Status*>统计发往每个host的请求的结果。
* METHOD_STATUS_MAP map<string,mapwithlock>统计发往每个host的调用{service，method}方法的请求结果。这里map<key,map>的结构考虑线程安全和锁定颗粒度不能太粗，整体用一把锁保护，内部的每一个map在对应一把锁定义为mapwithlock结构。访问时先获得外层的锁，找到key对应的mapwithlock，释放外层锁。然后获取mapwithlock的锁，在访问内层map的内容。（find需要遍历整个红黑树，而insert会改变红黑树结构，所以只要有可能用到读写两种操作map一定要用锁保护）。由于mapwithlock中的mutex是不可复制的，所以持有mutex*来赋值，持有map*,降低拷贝时的复杂度。
* startcount 为host的{service，method}对应的Status结构中的unreplied加1。加入threshold防止溢出。
* endcount 根据收到reponse结果修改success/failed值。
startcount和endcount加到rpcchannel的callmethod和onrpcmessage内。

* leastunrepliedmethod 从可用的hostlist中选出调用{service,method}方法的unreplied最少的host返回，如果有多个结果并列，那么随机选择一个返回。

### Consistent_hash
哈希一致性算法。将每个可用的host计算hash作为节点插入map<hashval,host>中（同时插入一定的虚拟节点，在host后加入字符计算hash），然后对调用方法的参数计算哈希值hashcode（保证相同参数的请求发往同一主机，这样server端可以缓存结果，减少计算量），在map中寻找第一个大于hashcode的节点（如果没有就是map中最小的，也就是哈希值圆环环上的右侧第一个点），返回host值。 Hashfunc的Hash定义为纯虚函数，这里只提供了MD5hashfunc一种计算哈希值的实现。

* select为调用接口 map<string,Consistenthash*> selectors存放对于{service，method}方法的可用hostlist对象建的selector。如果hostlist改变了，需要重新建立selector（重新插入节点）。所以consistenthash中存有hostlist的指针，来判断是否是同一个hostlist否则新建selector。这里由redis查询返回的hostlist是一个不能修改的临时vector。







