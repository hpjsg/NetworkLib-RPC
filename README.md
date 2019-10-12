# A simple C++ based Network Library and RPC Framework


[![license](https://img.shields.io/github/license/mashape/apistatus.svg)](https://opensource.org/licenses/MIT)

  
## 总览  

本项目为C++11编写的轻量级网络库和简易RPC框架。
* 网络库实现了事件注册、事件通知模块及定时任务。
* 简易RPC框架提供了远程调用的同步和异步接口，使用Redis作为注册中心，并实现了客户端的负载均衡。  

## 编译环境  
```
* OS: Ubuntu 18.04
* Complier: g++ 7.4.0
```


## 第三方依赖
| 名称        | 作用         | 
| ------------- |:-------------:| 
| [google protobuf](https://developers.google.cn/protocol-buffers/)    |序列化反序列化 | 
|  [zlib](http://www.zlib.net/)  | 奇偶校验      |  
|  [hiredis](https://github.com/redis/hiredis)   | Redis注册中心客户端    |   
|  [openssl](https://www.openssl.org/)   | MD5计算Hash值    |   


## 技术要点
#### Network library
* 使用Epoll边沿触发的IO多路复用技术，非阻塞IO，使用Reactor模式
* 主线程只负责accept请求，并以Round Robin的方式分发给其它IO线程(兼计算线程)，锁的争用只会出现在主线程和某一特定线程中
* 使用eventfd实现了线程的异步唤醒
* 使用timerfd和红黑树管理定时器
* 使用智能指针等RAII机制进行资源管理防止内存泄漏
* 支持优雅关闭连接和服务器的主动关闭

#### RPC framework
* Redis作为注册中心，使用TCP server的事件注册和通知模块作为网络库实现Redis的异步调用
* 客户端部署多种负载均衡算法：Consistent Hash算法、LeastUnreplied算法
* 提供了多种调用接口sync和async(返回future结构)，支持注注册Callback函数
* 设计了基于Bounded Blockingqueue的Redis连接池以及针对客户端的RPC连接池，避免连接的频繁创建销毁的开销

 

## 代码统计

![cloc](https://github.com/hpjsg/server-minirpc/blob/master/datum/cloc.png)



