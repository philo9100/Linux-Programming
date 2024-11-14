#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

void* thread_func(void* arg)
{
    printf("thread_func--thread is running\n");

    // 获取线程的作用域
    int scope;              // 定义线程的作用域
    pthread_attr_t attr;    // 定义线程属性对象
    /*
    pthread_getattr_np函数是一个非标准的POSIX扩展函数，用于获取指定线程的线程属性对象。
    需要注意的是，pthread_getattr_np函数是一个非标准的扩展函数，其名称中的“_np”表示“non-portable”，即非可移植。因此，它并不是POSIX标准的一部分，不是所有的系统和库都支持这个函数。
    */
    // 获取当前线程属性对象
    pthread_getattr_np(pthread_self(), &attr);

    // 获取线程属性对象中的作用域
    pthread_attr_getscope(&attr, &scope);
     if (scope == PTHREAD_SCOPE_SYSTEM) {
        printf("Thread scope is set to system scope\n");
    } else if (scope == PTHREAD_SCOPE_PROCESS) {
        printf("Thread scope is set to process scope\n");
    }

    sleep(3);
    printf("thread_func--thread is exiting\n");
    pthread_exit(NULL);
}

int main()
{
    pthread_t mythread;     // 定义线程标识符
    pthread_attr_t attr;    // 定义线程属性对象
    int scope; // 定义线程的作用域

    // 初始化线程属性对象
    pthread_attr_init(&attr);

    // 获取线程属性对象中的作用域
    pthread_attr_getscope(&attr, &scope);

    // 打印线程属性对象中的作用域
    switch(scope)
    {
        case PTHREAD_SCOPE_SYSTEM:
            printf("Thread scope is PTHREAD_SCOPE_SYSTEM\n");
            break;
        case PTHREAD_SCOPE_PROCESS:
            printf("Thread scope is PTHREAD_SCOPE_PROCESS\n");
            break;
        default:
            fprintf(stderr, "Illegal scope value.\n");
            exit(EXIT_FAILURE);
    }

    // 设置线程属性对象中的作用域为PTHREAD_SCOPE_PROCESS
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_PROCESS);


    // 创建线程
    pthread_create(&mythread, &attr, thread_func, NULL);

    // 等待线程结束并回收线程
    pthread_join(mythread, NULL);

    // 销毁线程属性对象
    pthread_attr_destroy(&attr);

    printf("Main thread exiting\n");

    return 0;
}