# zyw_co
* 使用c/c++实现的一简易协程库与网络库。
* 支持epoll的rector事件驱动结合调度
* 支持以同步编程的方式异步执行
* 支持条件变量
* 优化上下文切换与上下文获取(去除不需要的浮点数保存以及信号掩码系统调用)
* 基于最小堆与epoll实现毫秒精度的定时器
* 基于符号覆盖原则实现hook部分函数:
read  write  send  recv  sendto  recvfrom  accept sleep
* 支持协程间的join,suspend,wait等同步手段.

