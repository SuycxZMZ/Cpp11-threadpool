#include <iostream>
#include <random>
#include <chrono>
#include <vector>
#include <numeric>
#include "parallel_accumulate.h"

// 线程池测试代码, 用并行累加算法测试

thread_local unsigned thread_pool::my_index = 1;
thread_local work_stealing_queue* thread_pool::local_work_queue = NULL;

int main(int argc, char *argv[])
{
    // 使用C++11随机数生成器生成1到20之间的随机整数
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 20);

    // 生成长度为 vec_length 的vector，并填充随机整数
    const unsigned vec_length = 10000000;
    std::vector<int> v(vec_length);
    for (int i = 0; i < v.size(); i++) {
        v[i] = dis(gen);
    }

    // 普通 accumulate 求和
    int sum1 = 0;
    auto start1 = std::chrono::high_resolution_clock::now();
    // std::accumulate 在标准库 numeric 中 
    sum1 = std::accumulate(v.begin(), v.end(), 0);
    auto end1 = std::chrono::high_resolution_clock::now();

    // 用带有线程池的 并行 accumulate 求和
    int sum2 = 0;
    auto start2 = std::chrono::high_resolution_clock::now();
    sum2 = parallel_accumulate(v.begin(), v.end(), 0);
    auto end2 = std::chrono::high_resolution_clock::now();


    // 输出求和结果和所花费的时间
    std::cout << " ========== Normal accumulate ========== " << std::endl;
    std::cout << "Sum: " << sum1 << std::endl;
    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1).count() << " ms" << std::endl;

    std::cout << " ========== Parallel accumulate ========== " << std::endl;
    std::cout << "Sum: " << sum2 << std::endl;
    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2).count() << " ms" << std::endl;

    // std::cout << "Hello world!" << std::endl;
}
