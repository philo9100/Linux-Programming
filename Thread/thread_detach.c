#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

void* thread_func(void* arg)
{
    printf("thread_func--thread is running\n");
    // 模拟线程执行工作
    sleep(3);
    printf("thread_func--thread is exiting\n");
    pthread_exit(NULL);
}

int main()
{
    pthread_t mythread;
    int ret;

    // 创建线程
    ret = pthread_create(&mythread, NULL, thread_func, NULL);
    if(ret != 0)
    {
        fprintf(stderr, "pthread_create error: %s\n", strerror(ret));
        exit(EXIT_FAILURE);
    }

    /*
    新线程在开始时就被设置为分离状态。这意味着当这个线程结束时，它的资源会被系统自动回收，而不需要主线程调用pthread_join()函数来回收。
    */
    // 分离线程
    ret = pthread_detach(mythread);
    if(ret != 0)
    {
        fprintf(stderr, "pthread_detach error: %s\n", strerror(ret));
        exit(EXIT_FAILURE);
    }
     printf("Thread has been detached.\n");

    // 主线程休眠3秒
    sleep(1);

    pthread_exit(NULL);
}

