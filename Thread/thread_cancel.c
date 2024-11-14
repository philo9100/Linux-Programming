#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

void *mythread_func(void *arg)
{
    void *retval;
    printf("new thread started\n");

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    for (int i = 0; i==0; ) {
        pthread_testcancel(); // 显示设置一个取消点

        printf("new thread is running\n");
    }

    pthread_exit(retval);
}



int main()
{
    pthread_t mythread; // thread identifier
    pthread_attr_t myattr; // thread attributes
    void *retval;    // return value

    pthread_attr_init(&myattr);

    pthread_create(&mythread, &myattr, mythread_func, NULL);

    sleep(1);

    // 发送取消请求，取消线程
    pthread_cancel(mythread);

    pthread_join(mythread, &retval);

    if(retval == PTHREAD_CANCELED)
        printf("thread was canceled\n");
    else
        printf("thread wasn't canceled\n");

    return 0;
}


