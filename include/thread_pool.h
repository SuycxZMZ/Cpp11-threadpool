#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <queue>
#include <future>
#include <memory>
#include <functional>
#include <iostream>
#include <vector>
#include <thread>
#include "threadsafe_queue.h"
#include "work_steal_queue.h"

// join_threads : RAII 手法类, 确保无论以什么方式离开代码块, 所有线程最终都可以汇合
class join_threads
{
private:
    std::vector<std::thread>& threads;
public:
    explicit join_threads(std::vector<std::thread>& _threads) : threads(_threads)
    {}

    ~join_threads()
    {
        for (unsigned long i = 0; i < threads.size(); i++)
        {
            if (threads[i].joinable())
            {
                threads[i].join();
            }
        }
        
    }
};


class thread_pool
{
private:

    // 包装任务类, 在 work_steal_queue.h 中定义
    typedef function_wrapper task_type;

    // 数据成员声明顺序很重要
    std::atomic_bool done;

    // 线程池全局任务队列
    threadsafe_queue<task_type> pool_work_queue;

    // queues 的每个成员是指向各个线程的局部队列 ,  
    std::vector<std::unique_ptr<work_stealing_queue> > queues;

    // 池中的线程
    std::vector<std::thread> threads;

    // join_threads : RAII 手法类, 确保无论以什么方式离开代码块, 所有线程最终都可以汇合 
    join_threads joiner;

    typedef std::queue<task_type> local_queue_type;
    // 线程局部任务队列, 使用 work_stealing_queue* 节省空间, 
    // 不能用 std::unique_ptr<> , 因为线程池结构体还维护一个 queues , 其成员指向各个线程的局部队列
    static thread_local work_stealing_queue* local_work_queue;
    static thread_local unsigned my_index;

    // 线程执行函数
    void worker_thread(unsigned my_index_);

    // 从局部队列取任务
    bool pop_task_from_local_queue(task_type& task);

    // 从全局队列取任务
    bool pop_task_from_pool_queue(task_type& task);

    // 任务窃取
    bool pop_task_from_other_thread_queue(task_type& task);

public:

    thread_pool():
       done(false) ,joiner(threads)
    {

        // std::cout << "thread_pool start create ! " << std::endl;

        unsigned const thread_count=std::thread::hardware_concurrency();

        try
        {
            // 初始化 queues
            queues.reserve(thread_count);
            for(unsigned i=0;i<thread_count;++i)
            {
                queues.push_back(std::unique_ptr<work_stealing_queue>(
                    new work_stealing_queue));
            }

            // 初始化 threads
            threads.reserve(thread_count);
            for(unsigned i=0;i<thread_count;++i)
            {
                threads.push_back(
                    std::thread(&thread_pool::worker_thread, this, i));
            }

        }
        catch(...)
        {
            done=true;
            throw;
        }

    }

    ~thread_pool()
    {
        //std::cout << "thread_pool destroy ! " << std::endl;
        done=true;
    }

    // 使用者可以等待池内任务完成
    // submit 返回一个future实列, 持有任务的返回值, 调用者凭借该返回值等待任务完成
    template<typename FunctionType>
    std::future<typename std::result_of<FunctionType()>::type>
    submit(FunctionType f);

    void run_pending_task();
};

// 使用者可以等待池内任务完成
// submit 返回一个future实列, 持有任务的返回值, 调用者凭借该返回值等待任务完成
template<typename FunctionType>
std::future<typename std::result_of<FunctionType()>::type>
thread_pool::submit(FunctionType f)
{
    typedef typename std::result_of<FunctionType()>::type result_type;
    std::packaged_task<result_type()> task(std::move(f));
    std::future<result_type> res(task.get_future());

    // 判断当前线程是否有任务队列, 有则添加到局部队列, 否则添加到全局队列
    if (local_work_queue)
    {
        local_work_queue->push(std::move(task));
    }
    else
    {
        pool_work_queue.push(std::move(task));
    }
    
    return res;
}

#endif