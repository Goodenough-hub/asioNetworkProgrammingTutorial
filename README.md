# asioNetworkProgrammingTutorial

C++ asio网络编程教程，从实战角度讲述如何使用asio网络库搭建高可用服务器。 

- [博客地址](https://llfc.club/)
- [源码地址](https://gitee.com/secondtonone1/boostasio-learn)

项目顺序：
1. boosttest——visual studio配置boost库
2. endpoint——asio 终端节点的创建，socket的创建与连接，buffer缓冲结构
3. SyncClient/SyncServer——同步读写客户端与服务器端
4. AsycApi——异步读写api
5. AsycServer——异步echo服务器。存在的隐患：二次析构问题。读/写操作失败都会delete，可能导致程序奔溃。`core dump`。
   1. 官方源码中是echo方式，只绑定服务器的读事件，不会出现这种问题。但是实际项目中是全双工通信，很大可能出现这种问题。
6. AsycServer——利用C11模拟伪闭包实现连接的安全回收,智能指针`shared_ptr<Session>`，它的引用计数至少是三，map里引用计数要加一，在生成一个新的读回调函数的时候要加一，在生成一个新的写的回调函数的时候也会加一。这种方式在实际生产中会遇到。