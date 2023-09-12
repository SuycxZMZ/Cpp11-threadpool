#include <iostream>
#include <random>
#include <chrono>
#include <vector>
#include <numeric>
#include "../include/parallel_qsort.h"
#include "../include/parallel_accumulate.h"

// 线程池测试代码, 用 并行累加算法 和 并行快速排序 测试
thread_local unsigned thread_pool::my_index = 1;
thread_local work_stealing_queue* thread_pool::local_work_queue = NULL;

void test_accumulate(std::vector<int>&& test_vec);

void test_qsort(std::list<int>&& test_list);

int main(int argc, char *argv[])
{
    // 使用C++11随机数生成器生成1到20之间的随机整数
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 20);

    // 生成长度为 vec_length 的vector，并填充随机整数
    const unsigned vec_length = 10000000;
    const unsigned list_length = 10000;
    std::vector<int> v(vec_length);
    std::list<int> l(list_length);
    for (int i = 0; i < vec_length; i++) {
        v[i] = dis(gen);
    }

    for (int i = 0; i < list_length; i++) {
        l.push_back(dis(gen));
    }

    test_accumulate(std::move(v));
    test_qsort(std::move(l));

}

void test_accumulate(std::vector<int>&& test_vec)
{
    int sum1 = 0;
    auto start1 = std::chrono::high_resolution_clock::now();
    sum1 = std::accumulate(test_vec.begin(), test_vec.end(), 0);
    auto end1 = std::chrono::high_resolution_clock::now();

    // 输出求和结果和所花费的时间
    std::cout << " ========== std::accumulate ========== " << std::endl;
    std::cout << "Sum: " << sum1 << std::endl;
    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1).count() << " ms" << std::endl;

    // 用带有线程池的 并行 accumulate 求和
    int sum2 = 0;
    auto start2 = std::chrono::high_resolution_clock::now();
    sum2 = parallel_accumulate(test_vec.begin(), test_vec.end(), 0);
    auto end2 = std::chrono::high_resolution_clock::now();
    std::cout << " ========== parallel_accumulate ========== " << std::endl;
    std::cout << "Sum: " << sum2 << std::endl;
    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2).count() << " ms" << std::endl;
}

void test_qsort(std::list<int>&& test_list)
{
    auto start1 = std::chrono::high_resolution_clock::now();
    test_list.sort();
    auto end1 = std::chrono::high_resolution_clock::now();

    // 输出求和结果和所花费的时间
    std::cout << " ========== list<int>.sort() ========== " << std::endl;
    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1).count() << " ms" << std::endl;

    // 用带有线程池的 并行 accumulate 求和
    auto start2 = std::chrono::high_resolution_clock::now();
    parallel_quick_sort<int>(test_list);
    auto end2 = std::chrono::high_resolution_clock::now();
    std::cout << " ========== parallel_accumulate ========== " << std::endl;
    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2).count() << " ms" << std::endl;
}

