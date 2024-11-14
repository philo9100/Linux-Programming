#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

typedef struct
{
    int a;
    int b;
}exit_t;

void* thread_func(void* arg)
{
    exit_t* ret = (exit_t*)malloc(sizeof(exit_t));
    ret->a = 100;
    ret->b = 200;

    // 如果当前线程执行pthread_exit()函数，则会退出当前线程，并将线程执行函数的返回值通过pthread_exit()函数返回
    pthread_exit((void*)ret);
    return NULL;    // 测试线程执行函数的返回值是否会被pthread_exit()函数返回
}

int main()
{
    pthread_t mythread;
    exit_t* retval = NULL;

    pthread_create(&mythread, NULL, thread_func, NULL);

    /*
    retval是个一级指针，通过取地址并强制转换成void类型的二级指针传入pthread_join()函数中，在pthread_join()函数中通过取值操作访问到一级指针内存空间即retval变量映射的内存空间，因为操作的是二级指针往回取值访问的内存空间大小是固定的，将pthread_exit()函数保存的线程执行函数的返回值写入到retval的内存空间中，即线程执行函数中exit_t类型结构体的地址。
    */
    pthread_join(mythread, (void**)&retval);

    // 这里main线程知道retval是exit_t类型的指针，所以可以直接通过retval指针访问到结构体中的成员
    printf("a = %d, b = %d\n", retval->a, retval->b);

    return 0;
}