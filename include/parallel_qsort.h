#ifndef PARALLEL_QSORT_H
#define PARALLEL_QSORT_H

#include <list>
#include <algorithm>
#include <vector>
#include "thread_pool.h"


// soter 模板类 包装 thread_pool 实列,简化代码
template<typename T>
struct sorter
{
    thread_pool pool;
    
    std::list<T> do_sort(std::list<T>& chunk_data)
    {
        if(chunk_data.empty())
        {
            return chunk_data;
        }
        
        // 切掉第一个元素当作中枢
        std::list<T> result;
        result.splice(result.begin(),chunk_data,chunk_data.begin());
        T const& partition_val=*result.begin();
    
        // 划分
        typename std::list<T>::iterator divide_point=
            std::partition(
                chunk_data.begin(),chunk_data.end(),
                [&](T const& val){return val<partition_val;});

        // 存储前半部分
        std::list<T> new_lower_chunk;
        new_lower_chunk.splice(
            new_lower_chunk.end(),
            chunk_data,chunk_data.begin(),
            divide_point);

        // 前半部分排序任务提交给线程池
        std::future<std::list<T> > new_lower=
            pool.submit(
                std::bind(
                    &sorter::do_sort,this,
                    std::move(new_lower_chunk)));

        // 后半部分正常递归
        std::list<T> new_higher(do_sort(chunk_data));
        
        // 拼接后半部分
        result.splice(result.end(),new_higher);
        
        // 阻塞等待前半部分排序完成
        result.splice(result.begin(),new_lower.get());
        return result;
    }
};


template<typename T>
std::list<T> parallel_quick_sort(std::list<T> input)
{
    if(input.empty())
    {
        return input;
    }
    sorter<T> s;
    
    return s.do_sort(input);
}


#endif