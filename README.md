# net: A Header-Only Tcp Network Library

net是一个仅包含头文件的高性能TCP网络库。

net基于Reactor模式，采用的是MainReactor + SubReactorPool(每个线程一个SubReactor)的设计。

> net总体上参照了muduo的设计，同时也参考了trpc-cpp的一些设计。

## 项目组织

* docs: 文档
* examples: 使用示例
* include: 头文件
* test: 单元测试
* third_party: 第三方库

## 性能测试

在实现net之前，我曾经跟着陈硕的教程编写过一个miniMuduo网络库，但是该网络库具有如下缺点：

* 日志库性能不高。
* 在不少地方使用了锁。
* 保存了一些没必要保存的数据。
* 实现Reactor模式时，代码之间耦合比较严重，包含有很多的前置声明，可读性不是很好。

因此在net中，我针对这些点进行了优化：

* 使用了更高性能的[spdlog](https://github.com/gabime/spdlog) 日志库。
* 通过使用[concurrentqueue](https://github.com/cameron314/concurrentqueue) 中的MPMC无锁队列，并且借助thread local变量，让网络库部分完全无锁。
* 尽量不重复存储数据，并且实现了一个ObjectPool来提高内存利用率。
* net网络库中没有任何前置声明，只包含头文件，并且模块之间尽量解耦。

net的最终性能差不多是miniMuduo的两倍，与asio的性能不相上下。性能测试数据请参考[benchmark](docs/benchmark.md) 。

## 功能

- [x] TcpServer
- [x] TcpClient
- [x] HttpServer
- [x] HttpRoute
- [ ] HttpFileServer
- [ ] HttpsServer
- [ ] RpcServer
- [ ] FtpServer

