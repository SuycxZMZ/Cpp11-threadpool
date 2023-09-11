#include <vector>
#include <future>
#include <thread>
#include "thread_pool.h"

// 块累加
template<typename Iterator,typename T>
struct accumulate_block
{
    T operator()(Iterator first,Iterator last)
    {
        return std::accumulate(first,last,T());
    }
};

// 并行累加算法
template<typename Iterator,typename T>
T parallel_accumulate(Iterator first,Iterator last,T init)
{
    unsigned long const length=std::distance(first,last);

    if(!length)
        return init;

    // 任务数为物理线程数的2倍
    // 划分为 num_blocks 个任务
    unsigned const thread_count=std::thread::hardware_concurrency();
    unsigned long const num_blocks = 2 * thread_count;

    // 每个任务计算 block_size 的长度
    unsigned long const block_size = length / num_blocks;

    // 前 num_blocks-1 个任务的结果
    std::vector<std::future<T> > futures(num_blocks-1);

    // 用线程池计算
    thread_pool pool;

    Iterator block_start=first;
    for(unsigned long i=0;i<(num_blocks-1);++i)
    {
        // 开始划分
        Iterator block_end=block_start;
        std::advance(block_end,block_size);

        // 提交一个任务
        futures[i]=pool.submit( [=] {
            return accumulate_block<Iterator, T>() (block_start, block_end);
        } );

        // block_start 前进, 准备下一个任务
        block_start=block_end;
    }

    // 计算最后一块
    T last_result=accumulate_block<Iterator, T>() (block_start,last);

    // 把所有块的结果求和
    T result=init;
    for(unsigned long i=0;i<(num_blocks-1);++i)
    {
        result+=futures[i].get();
    }
    result += last_result;
    return result;
}
