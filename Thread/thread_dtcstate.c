#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

void* thread_func(void* arg)
{
    printf("thread_func--thread is running\n");
    sleep(3);
    printf("thread_func--thread is exiting\n");
    pthread_exit(NULL);
}

int main()
{
    pthread_attr_t attr;    // 定义线程属性对象
    pthread_t mythread;     // 定义线程标识符
    int detachstate;        // 定义线程的分离状态
    int ret;                // 定义返回值

    // 初始化线程属性对象
    pthread_attr_init(&attr);

    // 获取线程的分离状态
    ret = pthread_attr_getdetachstate(&attr, &detachstate);
    if(ret != 0)
    {
        fprintf(stderr, "pthread_attr_getdetachstate error: %s\n", strerror(ret));
        exit(EXIT_FAILURE);
    }

    // 打印线程的分离状态
    if(detachstate == PTHREAD_CREATE_JOINABLE)
    {
        printf("thread is PTHREAD_CREATE_JOINABLE\n");
    }
    else if(detachstate == PTHREAD_CREATE_DETACHED)
    {
        printf("thread is PTHREAD_CREATE_DETACHED\n");
    }

    // 设置线程的分离状态为PTHREAD_CREATE_DETACHED
    ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if(ret != 0)
    {
        fprintf(stderr, "pthread_attr_setdetachstate error: %s\n", strerror(ret));
        exit(EXIT_FAILURE);
    }

    // 获取线程的分离状态
    ret = pthread_attr_getdetachstate(&attr, &detachstate);
    if(ret != 0)
    {
        fprintf(stderr, "pthread_attr_getdetachstate error: %s\n", strerror(ret));
        exit(EXIT_FAILURE);
    }

    // 打印线程的分离状态
    if(detachstate == PTHREAD_CREATE_JOINABLE)
    {
        printf("thread is PTHREAD_CREATE_JOINABLE\n");
    }
    else if(detachstate == PTHREAD_CREATE_DETACHED)
    {
        printf("thread is PTHREAD_CREATE_DETACHED\n");
    }

    // 创建线程
    ret = pthread_create(&mythread, &attr, thread_func, NULL);
    if(ret != 0)
    {
        fprintf(stderr, "pthread_create error: %s\n", strerror(ret));
        exit(EXIT_FAILURE);
    }

    // 销毁线程属性对象
    pthread_attr_destroy(&attr);

    // 因为线程已经设置为分离状态，所以不需要调用pthread_join()函数来回收线程

    printf("Main thread exiting\n");

    // 由于子线程处于分离状态，它在完成后会自动清理资源
    return 0;
}