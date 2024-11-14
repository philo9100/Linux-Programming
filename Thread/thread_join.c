#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

#define NUM_THREADS 5 // 定义线程数量

// 线程执行函数
void* thread_func(void* arg)
{
    int thread_num = *(int*)arg;

    printf("thread %d func--thread_num = %d\n",thread_num, thread_num);

    // 模拟工作
    sleep(1);

    printf("Thread %d func--thread_num = %d is exiting\n", thread_num, thread_num);

    // 动态申请内存空间返回堆空间地址
    int* retval = (int*)malloc(sizeof(int));

    *retval = thread_num * 2; // 假设线程执行函数的返回值是线程号乘以2

    pthread_exit((void*)retval); // 退出线程
}

int main()
{
    pthread_t mythread[NUM_THREADS]; // 线程标识符数组
    int thread_num[NUM_THREADS]; // 线程号数组
    int ret;

    // 创建线程
    for(int i = 0; i < NUM_THREADS; ++i)
    {
        thread_num[i] = i; // 线程号数组作为每个线程的参数
        ret = pthread_create(&mythread[i], NULL, thread_func, (void*)&thread_num[i]);
        if(ret != 0)
        {
            fprintf(stderr, "pthread_create error: %s\n", strerror(ret));
            exit(EXIT_FAILURE);
        }
    }

    // 等待线程结束并回收线程
    for(int i = 0; i < NUM_THREADS; ++i)
    {
        int* retval = NULL;
        ret = pthread_join(mythread[i], (void**)&retval);
        if(ret != 0)
        {
            fprintf(stderr, "pthread_join error: %s\n", strerror(ret));
            exit(EXIT_FAILURE);
        }
        printf("main thread--thread %d's retval = %d\n", i, *retval);
        free(retval); // 释放内存
    }

    // 所有线程执行完毕后已回收，打印提示信息
    printf("main thread--All threads are exited\n");
    
    return 0;
}