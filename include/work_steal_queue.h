#ifndef WORK_STEAL_QUEUE_H
#define WORK_STEAL_QUEUE_H

#include <memory>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <deque>

// 设计 function_wrapper 的原因: packaged_task的实例只能移动不能复制, std::function 要求本身所含的函数对象可以拷贝构造
// 任务包装类, 可以包装任何可调用对象, 对外消除对象类型
class function_wrapper
{
    struct impl_base {
        virtual void call()=0;
        virtual ~impl_base() {}
    };

    std::unique_ptr<impl_base> impl;
    template<typename F>
    struct impl_type: impl_base
    {
        F f;
        impl_type(F&& f_): f(std::move(f_)) {}
        void call() { f(); }
    };
public:
    function_wrapper()
    {}

    template<typename F>
    function_wrapper(F&& f):
        impl(new impl_type<F>(std::move(f)))
    {}

    // 添加函数调用操作符
    void operator()() { impl->call(); }

    function_wrapper(function_wrapper&& other):
        impl(std::move(other.impl))
    {}

    function_wrapper& operator=(function_wrapper&& other)
    {
        impl=std::move(other.impl);
        return *this;
    }

    function_wrapper(const function_wrapper&)=delete;
    function_wrapper(function_wrapper&)=delete;
    function_wrapper& operator=(const function_wrapper&)=delete;
};

// work_stealing_queue 
// 对 std::deque<function_wrapper> 简单包装, push和try_pop 都操作队列前端, try_steal操作后端
class work_stealing_queue
{
private:
    typedef function_wrapper data_type;
    std::deque<data_type> the_queue;
    mutable std::mutex the_mutex;
    
public:
    work_stealing_queue()
    {}

    work_stealing_queue(const work_stealing_queue& other)=delete;
    work_stealing_queue& operator=(
        const work_stealing_queue& other)=delete;

    void push(data_type data);

    bool empty() const;

    bool try_pop(data_type& res);

    bool try_steal(data_type& res);
};


#endif