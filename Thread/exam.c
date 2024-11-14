#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */
#include <sys/types.h>
#include <string.h>
#include <errno.h>

void *mythread_func(void *arg)
{
    pthread_t mythread; // thread identifier
    pid_t tid, pid;

    int retval_1 = 666; // return value
    int *rerval_2 = (int*)malloc(sizeof(int));
    *rerval_2 = 777; 

    tid = syscall(SYS_gettid);
    pid = getpid();

    mythread = *(pthread_t*)arg;

    printf("mythread--getpid = %d, gettid = %d, pthread_self = %lu, pthread_t = %lu, arg = %ld\n", pid, tid, pthread_self(), (unsigned long)pthread_self(), mythread);

    //pthread_exit(&retval_1); // 函数出栈，retval_1的地址不可用, 如果返回该地址，会出现脏数据
    pthread_exit(rerval_2);
}


int main()
{
    pthread_t mythread; // thread identifier
    pthread_attr_t myattr; // thread attributes
    pid_t tid, pid;
    int ret;    // return value

    if(ret = pthread_attr_init(&myattr) != 0)
    {
        ret = errno;
        fprintf(stderr, "pthread_attr_init error: %s\n", strerror(ret));
        exit(EXIT_FAILURE);
    }

    pthread_create(&mythread, &myattr, mythread_func, &mythread);

    tid = syscall(SYS_gettid);
    pid = getpid();

    printf("main--getpid = %d, gettid = %d, pthread_self = %lu, pthread_t = %lu\n", pid, tid, pthread_self(), (unsigned long)pthread_self());

    int *myfunc_retval;   
    ret = pthread_join(mythread, &myfunc_retval);
    if(ret != 0)
    {
        ret = errno;
        fprintf(stderr, "pthread_join error: %s\n", strerror(ret));
        exit(EXIT_FAILURE);
    }

    printf("myfunc_retval = %d\n", *myfunc_retval);
    return 0;
}