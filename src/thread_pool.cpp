#include "thread_pool.h"


// 线程执行函数
void thread_pool::worker_thread(unsigned my_index_)
{

    // 池中的线程先初始化局部任务队列, 再进入处理任务循环
    my_index = my_index_;
    local_work_queue = queues[my_index].get();

    while(!done)
    {
        run_pending_task();
    }
}

// 从局部队列取任务
bool thread_pool::pop_task_from_local_queue(task_type& task)
{
    return local_work_queue && local_work_queue->try_pop(task);
}

// 从全局队列取任务
bool thread_pool::pop_task_from_pool_queue(task_type& task)
{
    return pool_work_queue.try_pop(task);
}

// 任务窃取
bool thread_pool::pop_task_from_other_thread_queue(task_type& task)
{
    // std::cout << "pop_task_from_other_thread_queue" << std::endl;

    for (unsigned i = 0; i < queues.size(); ++ i)
    {
        unsigned const index = (my_index + i + 1) % queues.size();

        if (queues[index]->try_steal(task))
        {
            return true;
        }
    }
    // std::cout << "No task ! " << my_index << std::endl;
    return false;
}


void thread_pool::run_pending_task()
{

    task_type task;
    if (pop_task_from_local_queue(task) ||
        pop_task_from_pool_queue(task)  ||
        pop_task_from_other_thread_queue(task)
    )
    {
        task();
    }
    else
    {
        // std::cout << " taskqueue empty ! " << std::endl;
        std::this_thread::yield();
    }
}

