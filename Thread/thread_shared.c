#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int var = 100; // 全局变量

void *thread_fun(void *arg)
{
    var = 200; // 修改全局变量
    printf("mythread--thread_fun var = %d\n", var); // thread_fun--var = 200
    sleep(1);
    return (void*)0;
}

int main()
{
    printf("main thread--At first var = %d\n", var); // At first var = 100

    pthread_t mythread; // 线程标识符

    pthread_create(&mythread, NULL, thread_fun, NULL);  // 创建线程，执行线程函数

    sleep(2);

    printf("main thread--After thread_fun() var = %d\n", var); // After thread_fun() var = 200

    return 0;
}
