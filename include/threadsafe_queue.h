#ifndef THREADSAFE_QUEUE_H
#define THREADSAFE_QUEUE_H

#include <memory>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <deque>


// threadsafe_queue
// 尾插法, 以链表作为基础结构, 尾指针指向虚拟尾节点
// 采用较细粒度的锁, 增加并发性, 比如在push() 在没有持锁的情况下可以先进行内存分配, 多个线程可以并发执行
// 每个执行push() 的线程只有在指针赋值的阶段持锁, 若基础数据结构是 std::queue 则内存分配也要加锁

template<typename T>
class threadsafe_queue
{
private:
    struct node
    {
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;
    };
    
    std::mutex head_mutex;
    std::unique_ptr<node> head;
    std::mutex tail_mutex;
    node* tail;
    std::condition_variable data_cond;

    // 定义一些辅助函数，简化代码
    node* get_tail();
    std::unique_ptr<node> pop_head();
    std::unique_lock<std::mutex> wait_for_data();
    std::unique_ptr<node> wait_pop_head();
    std::unique_ptr<node> wait_pop_head(T& value);
    std::unique_ptr<node> try_pop_head();
    std::unique_ptr<node> try_pop_head(T& value);
public:
    // 接口
    threadsafe_queue():
        head(new node),tail(head.get())
    {}
    threadsafe_queue(const threadsafe_queue& other)=delete;
    threadsafe_queue& operator=(const threadsafe_queue& other)=delete;

    std::shared_ptr<T> try_pop();

    bool try_pop(T& value);


    // 支持等待功能的 wait_and_pop()
    std::shared_ptr<T> wait_and_pop();
    void wait_and_pop(T& value);
    void push(T new_value);
    bool empty();
};



// =================== 接口实现 =================== //
template<typename T>
typename threadsafe_queue<T>::node* threadsafe_queue<T>::get_tail()
{
    std::lock_guard<std::mutex> tail_lock(tail_mutex);
    return tail;
}

template<typename T>
std::unique_ptr<typename threadsafe_queue<T>::node> threadsafe_queue<T>::pop_head()
{
    std::unique_ptr<node> old_head=std::move(head);
    head=std::move(old_head->next);
    return old_head;
}

template<typename T>
std::unique_lock<std::mutex> threadsafe_queue<T>::wait_for_data()
{
    std::unique_lock<std::mutex> head_lock(head_mutex);
    // 如果队列中没有数据,lambda式返回false, head_lock将解锁, 调用的线程进入阻塞状态
    data_cond.wait(head_lock,[&]{return head!=get_tail();});
    // 传递锁,确保头节点弹出的过程中持有的是同一个锁
    return std::move(head_lock);
}

template<typename T>
std::unique_ptr<typename threadsafe_queue<T>::node> threadsafe_queue<T>::wait_pop_head()
{
    std::unique_lock<std::mutex> head_lock(wait_for_data());
    return pop_head();
}

template<typename T>
std::unique_ptr<typename threadsafe_queue<T>::node> threadsafe_queue<T>::wait_pop_head(T& value)
{
    std::unique_lock<std::mutex> head_lock(wait_for_data());
    // 指针的操作在持锁范围之外, 提高并发性
    value=std::move(*head->data);
    return pop_head();
}

template<typename T>
std::unique_ptr<typename threadsafe_queue<T>::node> threadsafe_queue<T>::try_pop_head()
{
    std::lock_guard<std::mutex> head_lock(head_mutex);
    if(head.get()==get_tail())
    {   // 测试不过直接返回
        return std::unique_ptr<node>();
    }
    return pop_head();
}

template<typename T>
std::unique_ptr<typename threadsafe_queue<T>::node> threadsafe_queue<T>::try_pop_head(T& value)
{
    std::lock_guard<std::mutex> head_lock(head_mutex);
    if(head.get()==get_tail())
    {
        return std::unique_ptr<node>();
    }
    value=std::move(*head->data);
    return pop_head();
}



template<typename T>
std::shared_ptr<T> threadsafe_queue<T>::try_pop()
{
    std::unique_ptr<node> const old_head=try_pop_head();
    return old_head?old_head->data:std::shared_ptr<T>();
}

template<typename T>
bool threadsafe_queue<T>::try_pop(T& value)
{
    std::unique_ptr<node> const old_head=try_pop_head(value);
    return old_head? true : false;
}

// 支持等待功能的 wait_and_pop()
template<typename T>
std::shared_ptr<T> threadsafe_queue<T>::wait_and_pop()
{
    std::unique_ptr<node> const old_head=wait_pop_head();
    return old_head->data;
}

template<typename T>
void threadsafe_queue<T>::wait_and_pop(T& value)
{
    std::unique_ptr<node> const old_head=wait_pop_head(value);
}

template<typename T>
void threadsafe_queue<T>::push(T new_value)
{
    std::shared_ptr<T> new_data(
        std::make_shared<T>(std::move(new_value)));
    std::unique_ptr<node> p(new node);

    // RAII手法对尾结点加锁
    {
        std::lock_guard<std::mutex> tail_lock(tail_mutex);
        tail->data=new_data;
        node* const new_tail=p.get();
        tail->next=std::move(p);
        tail=new_tail;
    }
    data_cond.notify_one();
}

template<typename T>
bool threadsafe_queue<T>::empty()
{
    std::lock_guard<std::mutex> head_lock(head_mutex);
    return (head==get_tail());
}

#endif