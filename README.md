### 一个基于C++11 的线程池

结合《C++并发编程实战》(第二版)中的代码片段，做了一个较为完整的线程池, 并且以带有线程池的并行累加算法作为测试。该线程池较为通用, 感兴趣的话可以移植到 webserver 项目中

**特点：**

使用线程安全的任务队列，并且用较为细粒度的锁提高并发性，支持等待功能

凭借持有 submit 返回的 future ，使用者可以等待池内的任务完成

支持线程局部队列，缓解高并发时的缓存乒乓问题

支持任务窃取，缓解任务分配不均

较为通用，使用者只需要向任务队列中提交可调用对像 

**一些说明：**

1，join_threads是一个 RAII 手法的类, 确保无论以什么方式离开代码块, 所有线程最终都可以汇合。它持有一个指向线程数组的引用，在析构时汇合数组中的所有线程。thread_pool 持有一个 join_threads 成员，并在构造函数中完成初始化。在 thread_pool 类中 join_threads 成员要在线程数组后面声明，确保在构造时已经有线程数组而析构时先汇合线程，再析构thread数组。

2，threadsafe_queue 作为线程池的全局任务队列，它是一个模板类。数据结构是自建单链表，尾插法,  尾指针指向虚拟尾节点。采用较细粒度的锁, 增加并发性, 比如在push() 在没有持锁的情况下可以先进行内存分配, 多个线程可以并发执行。

3，线程池中的每个线程都持有一个局部队列指针以及一个线程索引，类型都为 static thread_local ，这两个成员要在外部初始化。static thread_local 类型之后可以深究

4，设计 function_wrapper 类的原因: packaged_task的实例只能移动不能复制, std::function 要求本身所含的函数对象可以拷贝构造。function_wrapper 任务包装类, 可以包装任何可调用对象, 对外消除对象类型。任务窃取队列里的成员，以及线程池中实例化 threadsafe_queue 的类型都是 function_wrapper 。

**代码结构：**

.
├── Makefile
├── include
│   ├── parallel_accumulate.h
│   ├── thread_pool.h
│   ├── threadsafe_queue.h
│   └── work_steal_queue.h
├── lib
├── output
└── src
    ├── main.cpp
    ├── thread_pool.cpp
    └── work_steal_queue.cpp

main.cpp为测试代码，其他的可以直接拿走编译

**测试环境：**

ubuntu22.04 windows子系统

g++ 11.4

GNU Make 4.3

经测试用c++11到20标准都可以编译，makefile文件第10行可以更改











