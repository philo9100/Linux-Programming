#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

void* thread_func(void* arg)
{
    int policy; // 定义线程的调度策略
    struct sched_param param; // 定义线程的调度参数

    // 获取线程的调度策略和调度参数
    /*
    这个函数用于获取一个已经存在的线程的当前调度策略和调度参数。它可以在线程创建后的任何时刻调用，以查询线程的当前调度策略和参数。
    函数原型如下：
    int pthread_getschedparam(pthread_t thread, int *policy, struct sched_param *param);
    */
    pthread_getschedparam(pthread_self(), &policy, &param);

    if(policy == SCHED_OTHER)
        printf("Scheduling policy: SCHED_OTHER\n");
    else if(policy == SCHED_RR)
        printf("Scheduling policy: SCHED_RR\n");
    else if(policy == SCHED_FIFO)
        printf("Scheduling policy: SCHED_FIFO\n");

    printf("Scheduling priority: %d\n", param.sched_priority);

    /* 同时也有设置已经创建的线程的调度策略和调度参数的函数pthread_setschedparam()，这个函数通常用于动态改变线程的行为，例如提高一个处理紧急任务的线程的优先级。函数原型如下：
    int pthread_setschedparam(pthread_t thread, int policy, const struct sched_param *param);
    
    对于不同的调度策略，优先级的取值范围是不同的。
    */

    pthread_exit(NULL);
}

int main()
{
    pthread_t mythread;     // 定义线程标识符
    pthread_attr_t attr;    // 定义线程属性对象
    struct sched_param param; // 定义线程的调度参数
    int policy; // 定义线程的调度策略
    int ret;    // 定义返回值

    // 初始化线程属性对象
    pthread_attr_init(&attr);

    // 获取线程属性对象中的调度策略
    ret = pthread_attr_getschedpolicy(&attr, &policy);
    if(ret != 0)
    {
        fprintf(stderr, "pthread_attr_getschedpolicy error: %s\n", strerror(ret));
        exit(EXIT_FAILURE);
    }
    
    // 打印线程属性对象中的调度策略
    if(policy == SCHED_OTHER)
        printf("Scheduling policy: SCHED_OTHER\n");
    else if(policy == SCHED_RR)
        printf("Scheduling policy: SCHED_RR\n");
    else if(policy == SCHED_FIFO)
        printf("Scheduling policy: SCHED_FIFO\n");

    // 获取线程属性对象中的调度参数
    /*
    这个函数用于从线程属性对象（pthread_attr_t 类型）中获取调度参数，主要是线程的优先级。它通常用在创建线程之前，你可以通过此函数获取线程属性对象中当前设置的调度参数。
    */
    ret = pthread_attr_getschedparam(&attr, &param);
    if(ret != 0)
    {
        fprintf(stderr, "pthread_attr_getschedparam error: %s\n", strerror(ret));
        exit(EXIT_FAILURE);
    }

    // 打印线程属性对象中的调度参数
    printf("Scheduling priority: %d\n", param.sched_priority);

    // 设置线程属性对象中的调度策略为SCHED_FIFO
    ret = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    if(ret != 0)
    {
        fprintf(stderr, "pthread_attr_setschedpolicy error: %s\n", strerror(ret));
        exit(EXIT_FAILURE);
    }

    // 设置线程属性对象中的调度参数
    param.sched_priority = 10;

    ret = pthread_attr_setschedparam(&attr, &param);
    if(ret != 0)
    {
        fprintf(stderr, "pthread_attr_setschedparam error: %s\n", strerror(ret));
        exit(EXIT_FAILURE);
    }

    // 设置线程属性对象中的继承调度策略属性为PTHREAD_EXPLICIT_SCHED
    /*
    忽略了继承属性：默认情况下，POSIX线程库可能忽略线程属性对象中的调度策略和参数，除非你明确地启用了属性继承。
    要启用调度属性的继承，你需要调用 pthread_attr_setinheritsched
    如果没有调用这个函数，或者你将继承设置为 PTHREAD_INHERIT_SCHED，新创建的线程将继承其父线程的调度策略和参数，而不考虑 attr 中设置的值。
    */
    ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    if(ret != 0)
    {
        fprintf(stderr, "pthread_attr_setinheritsched error: %s\n", strerror(ret));
        exit(EXIT_FAILURE);
    }

    // 创建线程
    ret = pthread_create(&mythread, &attr, thread_func, NULL);
    
    if(ret != 0)
    {
        fprintf(stderr, "pthread_create error: %s\n", strerror(ret));
        exit(EXIT_FAILURE);
    }

    // 等待线程结束并回收线程
    pthread_join(mythread, NULL);

     // 销毁线程属性对象
    pthread_attr_destroy(&attr);

    printf("Main thread exiting\n");

    return 0;
}