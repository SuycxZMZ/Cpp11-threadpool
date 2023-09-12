### GDB追踪 segmentation fault 位置

#### 1. 设置在当前文件夹下生成core文件
```C++
// 查看系统资源限制
ulimit -a
// 修改core文件大小限制
ulimit -c unlimited
// core文件生成在当前目录
sudo bash -c 'echo core.%e.%p > /proc/sys/kernel/core_pattern'
```
#### 2. 调试
```C++
// 1. 生成core文件，确保可执行目标文件可调试。执行
./main
// 2. gdb 查看core文件
gdb main corefile
// 3. 简单的错误一般会直接给出，另外，也可以用 where 命令查询。
where
// 4. 使用 info 命令可以显示程序的状态，包括停止的线程和核心文件的大小
info program
// 5. 使用backtrace命令查看堆栈跟踪。这将显示在core文件生成时程序的状态和调用堆栈
backtrace
```

#### 3. 简单例子
```C++
#include <cstdio>

int func(int *p)
{
    // 不检查指针有效性
    int y = *p;
    return y;
}
int main()
{
    int *p = NULL;
    // 访问一下NULl
    return func(p);
}
```
```C++
// 简单错误
(gdb) where
#0  0x00005607659fe139 in func (p=0x0) at test.cpp:4
#1  0x00005607659fe163 in main () at test.cpp:10
```