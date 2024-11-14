# 线程

在Linux中，线程是轻量级的进程，它们共享同一个地址空间和其他资源，但是每个线程都有自己的栈和寄存器。在Linux中，所有的线程都是通过clone()系统调用创建的，这包括通过POSIX的线程库中的pthread_create()函数创建的线程。

当使用pthread_create()函数创建一个新的线程时，新的线程会在同一个进程（也就是线程组）中创建，这意味着新的线程和创建它的线程共享同一个线程组ID（TGID），这个TGID就是原来的进程ID（PID）。

在Linux中，每个线程都有一个唯一的线程ID（TID），你可以使用gettid()函数来获取它。但是，getpid()函数在任何线程中都会返回同一个值，即线程组ID（TGID）。PID通常指的是线程组ID（TGID）。

## 线程操作

### `pthread_create()`函数

> 在Linux中，创建线程的最底层接口是`clone()`系统调用。然而，`clone()`函数非常底层且复杂，直接使用它来创建线程需要处理许多细节。在实际编程中，我们通常使用更高级的接口`pthread_create()`来创建线程。`pthread_create()`函数是`POSIX`线程库提供的，它在内部使用`clone()`来创建线程，但是它为我们处理了许多底层细节，使得创建线程变得更简单。
>
> Linux系统中创建线程的系统调用接口为`pthread_create()`，该函数存在于函数库`pthread.h`中，其原型如下：
>
> ```c
> int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
> ```
>
> - `thread`：指向线程标识符的指针；
> - `attr`：用来设置线程属性；
> - `start_routine`：一个函数指针，指向线程执行函数的起始地址；> - `arg`：线程执行函数的参数。
> - `返回值`：线程创建成功，则返回0，否则返回出错编号。
>
> 在线程调用`pthread_create()`函数创建出新线程后，当前线程会从调用`pthread_create()`函数的位置继续往下执行，而新创建的线程则从`start_routine()`函数开始执行。若`pthread_create()`函数成功返回，新线程的`id`会被写入`thread`指向的内存单元中。
> `pthread_create()`函数执行成功后，新线程的`ID`（这是一个`pthread_t`类型的值）会被写入到`pthread_create()`的第一个参数指向的内存单元中。这个线程`ID`是`POSIX`线程库用来标识线程的，它和Linux内核的线程`ID（TID）`是不同的。可以使用`pthread_self()`函数来获取当前线程的`POSIX`线程ID，这个函数返回的值和`pthread_create()`函数返回的线程ID是相同的。
>
> 在Linux中，可以使用`gettid()`系统调用来获取当前线由内核维护的`ID（TID）`。这个TID是内核用来标识线程的，它和POSIX线程ID是不同的。
>
> `pthread_self()`函数存在于函数库`pthread.h`中，其原型如下：
>
> ```c
> pthread_t pthread_self(void);
> ```
>
> 线程的执行函数的原型如下：
>
> ```c
> void *start_routine(void *arg);
> ```
>
> `start_routine()`函数的参数`arg`是一个指向任意类型的指针，它指向的是`start_routine()`函数的参数。`start_routine()`函数的返回值是一个指向任意类型的指针，它指向线程的返回值。如果线程不需要返回值，则可以直接使用`return`语句退出线程，或者调用`pthread_exit()`函数退出线程。

#### pthread_create()函数程序示例

```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>


// 线程执行函数
void *thread_fun(void *arg)
{
    /*
    在Linux中，gettid()函数是一个系统调用，用于获取当前线程的线程ID（TID）。
    然而，gettid()并没有在glibc中提供，所以不能直接在程序中调用它。

    如果想在程序中获取线程ID，你可以使用syscall(SYS_gettid)来调用gettid()系统调用。
    syscall()函数是在unistd.h头文件中定义的，SYS_gettid是在sys/syscall.h头文件中定义的。
    */
    pid_t tid = syscall(SYS_gettid); // 获取线程ID
    printf("thread_fun--getpid = %d, gettid = %d, pthread_self = %lu, pthread_t = %lu, arg = %ld\n", getpid(), tid, pthread_self(), (unsigned long)pthread_self(), *(pthread_t*)arg); 
    // thread_fun--getpid = 28578, gettid = 28579, pthread_self = 140338495387392, pthread_t = 140338495387392, arg = 140338495387392
    /*
    从上面的输出结果可以看出，线程ID和线程标识符是不同的，线程ID是系统分配的，而线程标识符是pthread_create()函数返回的。
    线程的pid是线程组长的pid，线程组长的pid和线程组长的线程ID是相同的。
    线程的线程表标识符pthread_t和pthread_self()函数的返回值是相同的，都是调用pthread_create()函数时传入的参数。
    */
    sleep(1);
    return (void*)0;    // (void*)0表示返回值为0x00 00 00 00 = NULL
}

int main()
{
    pthread_t mythread; // 线程标识符
    pid_t tid = syscall(SYS_gettid); // 获取线程ID

    int ret = pthread_create(&mythread, NULL, thread_fun, &mythread); // 创建线程，执行线程函数

    if (ret != 0) // 创建线程失败
    {
        fprintf(stderr, "pthread_create error: %s\n", strerror(ret));
        exit(EXIT_FAILURE);  
    }

    printf("main--getpid = %d, gettid = %d, pthread_self = %lu, pthread_t = %lu\n", getpid(), tid, pthread_self(), (unsigned long)pthread_self());
    // main--getpid = 28578, gettid = 28578, pthread_self = 140338503907136, pthread_t = 140338503907136
    /*
    从上面的输出结果可以看出，主线程（线程组长）的pid和线程组长的线程ID是相同的也是线程组的tgid
    主线程有自己的线程标识符pthread_t，并且线程表标识符pthread_t是pthread_self()函数的返回值。
    */
    sleep(2);

    // 等待线程结束
    ret = pthread_join(mythread, NULL);
    if (ret != 0) // 等待线程失败
    {
        fprintf(stderr, "pthread_join error: %s\n", strerror(ret));
        exit(EXIT_FAILURE);  
    }
    
    return 0;
}
```

> 在Linux（以及其他现代操作系统）中，虚拟地址空间是一个进程可以访问的内存地址的集合。这个地址空间是虚拟的，意味着它并不直接对应物理内存中的实际地址。相反，操作系统和硬件会一起将虚拟地址转换为物理地址。
>
> 虚拟地址空间不仅仅是地址的集合，它还包含了这些地址的属性和它们如何映射到物理内存的信息。例如，每个虚拟地址都有一些属性，如是否可读、可写、可执行，是否已经分配内存等。此外，虚拟地址空间还包含了这些虚拟地址如何映射到物理内存的信息，这是通过页表来实现的。
>
> 虚拟地址空间的主要优点是它允许每个进程都有自己的一块连续的地址空间，这样每个进程就可以独立地运行，不会影响其他进程。此外，虚拟地址空间还可以提供内存保护，因为每个进程只能访问自己的地址空间，不能访问其他进程的地址空间。
> 在Linux内核中，虚拟地址空间是由mm_struct数据结构来表示的，这个数据结构定义在<linux/mm_types.h>头文件中。mm_struct包含了许多字段，用于描述虚拟地址空间的各种属性，如页表、内存区域（memory region）、内存政策等。
>
> 每个进程都有一个task_struct数据结构来表示，这个数据结构定义在<linux/sched.h>头文件中。task_struct中有一个mm字段，这个字段是一个指向mm_struct的指针，用于表示进程的虚拟地址空间。
>
> 以下是mm_struct和task_struct的部分定义：
>
> ```c
> struct mm_struct {
>    struct vm_area_struct *mmap; /* list of VMAs */ //虚拟内存区域（VMA）包含连续的一段地址范围。每个VMA都有一些属性，如是否可读、可写、可执行等。
>    pgd_t *pgd; /* page global directory */ // 用于将虚拟地址映射到物理地址
>    /* ... other fields ... */
> };
>
> struct task_struct {
>    /* ... other fields ... */
>    struct mm_struct *mm; /* main memory space */
>    /* ... other fields ... */
> };
> ```
>
> 在这个示例中，mm_struct的mmap字段是一个指向虚拟内存区域（VMA）的链表的指针，将一系列VMA链接在一起，这些VMA描述了虚拟地址空间中的各个内存区域。例如，一个VMA可以描述进程的代码段，另一个VMA可以描述进程的数据段，还有一个VMA可以描述进程的堆栈段等。每个VMA对应了一系列的页面。
> pgd字段是页全局目录的指针。task_struct的mm字段是一个指向mm_struct的指针，表示进程的主内存空间。
>
> 在Linux中，进程拥有独立的虚拟地址空间。当一个进程通过fork()系统调用创建一个新进程时，父进程和子进程在开始时会共享相同的物理内存页面，只有当其中一个进程试图修改这些页面时，操作系统才会创建一个新的页面副本。子进程会获得父进程虚拟地址空间的一个副本。这意味着父进程和子进程有各自独立的虚拟地址空间，它们之间的任何修改都不会影响对方。
>
> 然而，需要注意的是，虽然父进程和子进程的虚拟地址空间是独立的，但在fork()之后，它们的内容是相同的。这是因为fork()会复制父进程的虚拟地址空间到子进程。但是，当父进程或子进程试图修改其虚拟地址空间时，操作系统会使用一种称为写时复制（copy-on-write）的技术来创建所需页面的一个新的副本，从而保证父进程和子进程的虚拟地址空间是独立的。
>
> 在Linux中，线程共享同一个虚拟地址空间，这意味着它们可以访问相同的内存地址。这是因为线程是在同一个进程中创建的，它们共享同一个虚拟地址空间。因此，在一个线程中对内存的修改会影响到其他线程。

#### 线程共享内存示例

```c
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
```

### `pthread_exit()`函数

> 在Linux中，线程可以通过调用`pthread_exit()`函数来使单个线程退出。该函数位于函数库`pthread.h`中，其函数原型如下：
>
> ```c
> void pthread_exit(void *retval);
> ```
>
> - `retval`：一个指向任意类型的指针，它指向线程执行函数的返回值。如果线程不需要返回值，则可以直接使用return语句退出线程，或者调用pthread_exit()函数退出线程。
>
> **void\* retval的通常用法**  
> `pthread_exit()`函数的参数`void *retval`用于指向线程执行函数的返回值。这个返回值可以被其他线程通过`pthread_join()`函数获取。
>
> 当你创建一个线程并指定一个线程执行函数时，这个线程执行函数可以返回一个`void*`类型的指针，这个任何类型的指针同`pthread_exit()`函数的参数类型相同。当线程执行函数返回时，这个返回值可以被`pthread_exit()`函数接收，并作为线程的返回值。
>
> 其他线程可以调用`pthread_join()`函数来等待一个线程结束，并获取这个线程的返回值。`pthread_join()`函数的第二个参数是一个`void**`类型的指针，这个二级指针就可以指向`retval`这个指针，即获取一个线程执行函数结束并退出后指向的结果。

#### pthread_exit()函数程序示例

```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

void* thread_fun(void *arg)
{
    int* ptr = (int*)malloc(sizeof(int)); // 申请内存

    *ptr = 666; // 给内存赋值

    // 之前都是通过return (void*)0语句来退出线程，这里我们通过pthread_exit()函数来退出线程，并将返回值传递给其他线程
    pthread_exit((void*)ptr); // 退出线程
    // 如果线程不需要返回值，则可以直接使用return语句退出线程，或者调用pthread_exit()函数退出线程。
}

int main()
{
    pthread_t mythread; // 线程标识符

    int ret;
    if(ret = pthread_create(&mythread, NULL, thread_fun, NULL))
    {
         fprintf(stderr, "pthread_create error: %s\n", strerror(ret));
        exit(EXIT_FAILURE);  
    }

    // 当前main线程要获取mythread线程的返回值，即一个void*类型的指针，那么就在此声明一个void*类型的指针，用于接收mythread线程执行函数的返回值，而此时result的内存空间应该是空的，需要把result的地址传给pthread_join()函数，让pthread_join()函数通过result的地址把mythread线程的返回值写入到result内存空间中。这样result指向的地址就和mythread线程的返回值指向同一个地址。
    void* result; // 用于接收线程执行函数的返回值，等同于线程执行函数的返回值

    // 等待线程结束，并获取线程的返回值
    if(ret = pthread_join(mythread, &result))
    {
        fprintf(stderr, "pthread_join error: %s\n", strerror(ret));
        exit(EXIT_FAILURE);  
    }

    // 在main线程中打印mythread线程中结果
    printf("main thread--mythread's result = %d\n", *(int*)result); // main thread--mythread's result = 666

    free(result); // 释放内存

    return 0;
}
```

```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void* thread_func(void* arg)
{
    int* thread_num = (int*)arg;
    printf("thread_func--thread_num = %d\n", *thread_num);

    //
    int* retval = (int*)malloc(sizeof(int));
    *retval = *thread_num + 100;

    pthread_exit((void*)retval);
}

int main()
{
    pthread_t mythread;
    int thread_num = 100;
    int ret;

    ret = pthread_create(&mythread, NULL, thread_func, (void*)&thread_num);
    if(ret != 0)
    {
        fprintf(stderr, "pthread_create error: %s\n", strerror(ret));
        exit(EXIT_FAILURE);
    }

    void* result;

    ret = pthread_join(mythread, &result);
    if(ret != 0)
    {
        fprintf(stderr, "pthread_join error: %s\n", strerror(ret));
        exit(EXIT_FAILURE);
    }

    printf("main thread--thread_func's result = %d\n", *(int*)result);

    free(result);

    return 0;
}
```

> 如果CPU只有一个核，那么在任何给定的时刻，只能有一个线程在执行。然而，操作系统会使用一种称为时间片轮转（time slicing）的技术，将CPU时间分割成很小的片段，并将这些时间片分配给各个线程。这样，虽然在任何给定的时刻只有一个线程在执行，但由于时间片很小，用户会感觉到多个线程似乎在同时执行，这就是并发执行。
>
> 如果CPU有多个核，那么可以有多个线程在同一时刻执行，这就是并行执行。在这种情况下，操作系统会尽可能地将线程分配到不同的CPU核上，以提高并行性和性能。
>
> 在线程执行函数中，使用pthread_exit()函数只会结束当前线程，这同使用return语句退出线程是一样的。并不会影响其他线程的执行，如果想要结束整个进程，可以使用exit()函数，这个函数会结束整个进程，包括所有的线程。

#### 分别使用pthread_exit()函数、return、exit()函数使其中一个线程退出，观察其他线程执行情况的程序示例

```c
/*
在这个示例中
thread_func1()通过调用pthread_exit()来退出线程
thread_func2()通过return来退出线程
thread_func3()通过调用exit()来退出整个进程。

你会注意到，当thread_func3()调用exit()时，整个进程都会立即结束，包括所有的线程。这就是为什么你不会看到"Main thread exiting"的输出。
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

void* thread1_func(void* arg)
{
    printf("thread1_func--thread1 is running\n");
    sleep(2);
    printf("thread1_func--thread1 is exiting\n");
    pthread_exit(NULL);
}

void* thread2_func(void* arg)
{
    printf("thread2_func--thread2 is running\n");
    sleep(2);
    printf("thread2_func--thread2 is exiting\n");
    return NULL;
}

void* thread3_func(void* arg)
{
    printf("thread3_func--thread3 is running\n");
    sleep(2);
    printf("thread3_func--thread3 is exiting\n");
    exit(EXIT_SUCCESS); // 退出整个进程
}

int main()
{
    pthread_t thread1, thread2, thread3;

    pthread_create(&thread1, NULL, thread1_func, NULL);
    pthread_create(&thread2, NULL, thread2_func, NULL);
    pthread_create(&thread3, NULL, thread3_func, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);

    printf("main thread--All threads are exited\n");

    return 0;
}
```

### `pthread_cancel()`函数

> 在线程操作中有一个与终止进程的函数`kill()`对应的系统调用`pthread_cancel()`，它可以用来向指定线程发送`CANCEL`信号终止一个线程。`pthread_cancel()`函数位于函数库`pthread.h`中，其函数原型如下：
>
> ```c
> int pthread_cancel(pthread_t thread);
> ```
>
> - `thread`：一个线程标识符，它指定了要终止的线程。
> - `返回值`：如果线程终止成功，则返回0，否则返回出错编号。当调用`pthread_cancel`函数取消一个线程时，被取消的线程会返回一个`PTHREAD_CANCELED`值，这是一个`void *`类型的值作为线程执行函数的返回值。
>
> 在多线程编程中，当请求取消一个线程时，取消操作不会立即发生，只有到达取消点时系统才会检测是否有未响应的取消信号，并对信号进行处理。所谓的取消点（cancellation point）是指线程在执行期间可以被取消的一个点。线程库定义了一些函数，这些函数可以被认为是取消点，即线程执行到这些函数时，如果收到了取消请求，它将响应请求并开始取消过程。
> 一些标准的取消点包括但不限于以下函数：
> sleep()、usleep()、nanosleep()、pthread_join()、pthread_cond_wait()、pthread_cond_timedwait()、pthread_testcancel()、read()、write()、open()、close()、ioctl()、select()、poll()、wait()、waitpid()、sem_wait()、getaddrinfo()、sigwait()
> 当一个线程通过执行上述函数中的某个函数进行阻塞操作时，如果其他线程向它发送取消请求，该线程将在执行这些函数时检查取消请求，并在其中断点执行取消操作，开始进行线程的清理工作。
>
> 线程可以通过调用pthread_setcancelstate来设置自己的取消状态，决定是启用取消（PTHREAD_CANCEL_ENABLE），还是禁用取消（PTHREAD_CANCEL_DISABLE）。此外，线程也可以通过调用pthread_setcanceltype来设置取消类型，是异步取消（PTHREAD_CANCEL_ASYNCHRONOUS）这意味着线程可以在任何时间被取消，而非仅在取消点，还是延迟取消（PTHREAD_CANCEL_DEFERRED），即只在取消点响应取消请求。
>
> pthread_testcancel函数是一个特殊的函数，它提供了一个显式的取消点，即使在当前执行点没有其他内置取消点。线程可以在逻辑上合适的时刻调用它，以确保及时响应取消请求。

#### pthread_cancel()函数程序示例

```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>


void* thread_func(void* arg)
{
    printf("New thread started\n");
    
    // 设置线程取消状态为PTHREAD_CANCEL_ENABLE，即启用取消
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

    // 设置线程取消类型为PTHREAD_CANCEL_DEFERRED，即延迟取消，只在取消点响应取消请求
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    for(int i = 0; i < 1000; ++i)
    {   
        // sleep(1); // sleep()是取消点 线程执行到这里时，如果收到了取消请求，它将响应请求并开始取消过程

        // 用pthread_testcancel()函数显示设置取消点，来检查是否有取消请求，如果有则响应取消请求
        pthread_testcancel();

        printf("thread_func--thread is running...\n");
    }

    pthread_exit(NULL);
}

int main()
{
    pthread_t mythread;
    void* retval = NULL;

    // 创建线程，执行线程函数
    pthread_create(&mythread, NULL, thread_func, NULL);

    // 主线程休眠3秒
    sleep(3);

    // 发送取消请求，取消线程
    pthread_cancel(mythread);

    // 等待线程结束
    pthread_join(mythread, &retval);

    // 打印线程退出码
    // 当调用pthread_cancel函数取消一个线程时，被取消的线程会返回一个PTHREAD_CANCELED值，这是一个void *类型的值
    if(retval == PTHREAD_CANCELED)
    {
        printf("main thread--thread is canceled\n");
    }
    else
    {
        printf("main thread--thread is not canceled\n");
    }
    return 0;
}
```

### `pthread_join()`函数

> 在Linux中，进程可以使用`wait()`、`waitpid()`系统调用将进程挂起来等待子进程结束。类似地，线程也可以使用`pthread_join()`函数来挂起线程等待其他线程结束。`pthread_join()`函数位于函数库`pthread.h`中，其函数原型如下：
>
> ```c
> int pthread_join(pthread_t thread, void **retval);
> ```
>
> - `thread`：一个线程标识符，它指定了要等待的线程。
> - `retval`：一个指向任意类型的指针，它指向线程执行函数的返回值。如果不关心线程的返回值，可以传递`NULL`。
>
> **调用该函数的线程将自己挂起并等待指定线程结束，需要注意以下几点：**  
>
> - 在同一个进程中创建的所有线程都共享同一个虚拟地址空间。调用`pthread_join()`的线程和它正在等待的线程必须是在同一个进程内，它们共享虚拟地址空间。
> - 多个线程不能同时对同一个线程调用`pthread_join()`。如果需要多个线程知道某个线程已经结束，可以使用条件变量、信号量或其他同步机制来实现。
> - 被等待的线程应该是可连接的（`joinable`）。默认情况下，线程是可连接的，但如果一个线程被设置成分离状态（`detached`）, 那么就不能对它调用`pthread_join()`。
> - 如果两个线程互相等待对方结束，则会发生死锁，这是因为它们都不会继续执行。因此，需要确保没有循环等待的情况出现。
> - 当一个线程结束时，它的退出状态保留在系统中直到另一个线程对它调用了`pthread_join()`，这样退出状态才会被清理。如果不调用`pthread_join()`，则该线程的状态信息可能会一直保留，导致资源泄漏（所谓的僵尸线程）。
> - `pthread_join()`的第二个参数是一个指针，用来存储结束线程的返回值。如果不关心线程的返回值，可以传递`NULL`。
> - 调用`pthread_join()`的线程会被挂起直到目标线程终止。在目标线程结束之前，调用线程不会继续执行。

#### 使用pthread_exit()函数退出线程，并将线程执行函数的返回值通过pthread_exit()函数返回的程序示例

```c
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
    // a = 100, b = 200，根据打印的结果可以看出，pthread_exit()函数成功使线程退出，线程执行函数的返回值通过pthread_exit()函数返回
    return 0;
}
```

#### 使用pthread_join()回收多个线程，并使用pthread_exit()函数函数的退出状态的程序示例

```c
/*
/* 
在这个程序中，首先定义了一个thread_func()线程执行函数，它接收一个指针作为参数，执行一些工作，然后使用pthread_exit()退出并返回一个指向整数的指针，该整数是线程编号的两倍。

在main()函数中，我们创建了NUM_THREADS个线程，并为每个线程传递了一个唯一的参数（它们的索引号）。然后我们使用pthread_join()等待每个线程结束并获取它们的返回值。由于工作线程通过malloc()分配返回值，因此在main()函数中使用完这些返回值后要释放它们。
*/
*/
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
```

#### `pthread_detach()`函数

> `pthread_detach()`函数可以将一个线程设置为分离状态将其从主控线程中分离，这样该线程结束时，系统会自动回收它的资源，而不需要其他线程调用`pthread_join()`函数来回收它的资源。`pthread_detach()`函数位于函数库`pthread.h`中，其函数原型如下：
>
> ```c
> int pthread_detach(pthread_t thread);
> ```
>
> - `thread`：一个线程标识符，它指定了要分离的线程。
> - `返回值`：线程分离成功，则返回0，否则返回出错编号。
>
> `thread_join()`函数不能用来回收分离状态的线程，否则会返回`EINVAL`错误。
>
> pthread_detach()函数的通常用法是将一个可join的线程设置为分离状态，这样一来，这个线程结束时会自动清理资源，包括线程描述符和栈。这对于某些类型的程序非常有用，尤其是那些创建了许多不需要与其他线程同步结束的线程的程序。在这些情况下，分离线程意味着创建者无需管理或回收这些线程。

通常用法
异步处理： 当你想让线程独立于其他线程运行时，例如后台处理或异步任务。

资源管理： 当线程无需与其他线程的结束进行同步，并且你希望线程能自行管理其结束和资源回收时。

避免僵尸线程： 如果不回收已结束的线程，线程的状态将一直保持在系统中，导致资源泄漏。分离线程能够自动清理，避免这种情况。

注意事项
状态无法获取： 一旦线程被分离，就不能使用pthread_join()来获取它的退出状态。如果你需要线程的退出信息，那么就不要将线程设置为分离状态。

二次分离： 不能对已经分离的线程再次调用pthread_detach()，这样做将导致未定义行为。

同一线程不能同时join和detach： 线程要么被join，要么被detach，不能两者都是。如果尝试对一个已经被join的线程调用pthread_detach()，或是对一个已经被detach的线程调用pthread_join()，都将返回错误。

分离自身： 线程可以自己调用pthread_detach(pthread_self())来分离自身。

及时分离： 分离操作最好在创建线程后尽快进行，以避免在线程结束和分离之间出现时间窗口，该时间窗口可能导致线程成为僵尸线程。

错误处理： 调用pthread_detach()后应该检查返回值，确保操作成功。

内存管理： 如果线程是动态分配的，需要确保在线程分离之前，管理这些资源的代码有适当的逻辑来释放它们。

总之，pthread_detach()用于那些不需要从其他线程中收集状态的线程，并且希望线程能够自我管理和清理。在决定分离一个线程之前，应当考虑到线程同步和资源管理等方面的需求。

#### pthread_detach()函数程序示例

```c
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
```

> 一旦线程被设置为分离状态，就不能再使用pthread_join()来获取其退出状态了。如果需要知道分离状态的线程何时结束或其结束状态，需要使用其他的通信机制，如全局变量、互斥量、条件变量、信号量等。
>
> 例如，可以在全局范围内定义一个变量，然后在线程函数中修改这个变量的值来表示线程的状态。主线程可以检查这个变量的值来知道线程何时结束。但是，这种方法需要使用互斥量来保护全局变量，以防止数据竞争。

#### 使用全局变量的方式来获取分离状态的线程的退出状态的程序示例

```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; // 初始化互斥量

int thread_status = 0; // 全局变量，用于表示线程的状态

void* thread_func(void* arg)
{
    printf("thread_func--thread is running\n");
    // 模拟线程执行工作
    sleep(3);
    printf("thread_func--thread is exiting\n");

    // 修改全局变量的值，表示线程已经结束
    pthread_mutex_lock(&lock);
    thread_status = 1;
    pthread_mutex_unlock(&lock);

    pthread_exit(NULL);
}

int main()
{
    pthread_t mythread;

    // 创建线程
    pthread_create(&mythread, NULL, thread_func, NULL);

    // 将新线程设置为分离状态
    pthread_detach(mythread);

    // 检查分离状态的线程是否结束
    while(1)
    {
        pthread_mutex_lock(&lock);
        if(thread_status == 1)
        {
            printf("Detached thread has finished.\n");
            pthread_mutex_unlock(&lock);
            break;
        }
        pthread_mutex_unlock(&lock);
        sleep(1);  // 等待一段时间再次检查
    }

    printf("Main thread exiting\n");

    return 0;
}
```

#### 使用回调函数的方式来获取分离状态的线程的退出状态的程序示例

```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

// int thread_num, int status
typedef void (*thread_exit_callback)(int, int); // 定义回调函数类型

// 全局互斥锁，用于同步对共享资源的访问
pthread_mutex_t lock;

// 线程回调函数
void on_thread_exit(int thread_num, int status)
{
    // 获取互斥锁
    pthread_mutex_lock(&lock);

    // 这里的代码是线程安全的，因为只有一个线程可以获取到互斥锁
    printf("Callback: Thread %d exited with status %d\n", thread_num, status);
    
    // 共享资源的更新或处理放在这里
    
    // 释放互斥锁
    pthread_mutex_unlock(&lock);
}

// 线程执行函数
void* thread_func(void* arg)
{
    thread_exit_callback callback =  (thread_exit_callback)arg; // 将arg强制转换成自定义回调函数类型

    pthread_t thread_self = pthread_self(); // 获取线程自身的线程标识符
    
    pthread_detach(thread_self); // 线程设置自分离状态，这也是pthread_detach()函数的通常用法，当然也可以在main()函数中调用pthread_detach()函数来设置线程分离状态

    // 将线程标识符当作线程的ID
    int thread_num = (int)thread_self; 

    printf("Thread %d is running\n", thread_num);

    // 模拟线程执行工作
    sleep(3);

    // 调用回调函数，并传递线程的状态
    callback(thread_num, thread_num * 2); // 
    
    // 退出线程
    pthread_exit(NULL);
}


int main()
{
    const int NUM_THREADS = 3; // 定义线程数量
    pthread_t mythread[NUM_THREADS]; // 线程标识符数组

    // 初始化互斥锁
    pthread_mutex_init(&lock, NULL);


    // 创建线程
    for(int i = 0; i < NUM_THREADS; ++i)
    {
        if(pthread_create(&mythread[i], NULL, thread_func, (void*)on_thread_exit) != 0)
        {
            perror("Failed to create the thread");
            // 清理互斥锁资源
            pthread_mutex_destroy(&lock);
            exit(EXIT_FAILURE);
        }
    }

    // 模拟主线程执行工作
    sleep(5);

    printf("Main thread exiting\n");

    // 销毁互斥锁
    pthread_mutex_destroy(&lock);

    return 0;
}
```

### 线程属性

> 使用`pthread_create()`函数创建线程时，可以通过传入参数`attr`来设置线程属性。该参数是一个指向`pthread_attr_t`结构体类型的指针，它指向一个线程属性对象，用于设置线程的属性。`pthread_attr_t`结构体类型位于函数库`pthread.h`中，其定义如下：
>
> ```c
> typedef struct
> {
>     int detachstate; // 线程的分离状态
>     int schedpolicy; // 线程调度策略
>     struct sched_param schedparam; // 线程的调度参数
>     int inheritsched; // 线程的继承性
>     int scope; // 线程的作用域
>     size_t guardsize; // 线程栈末尾的警戒缓冲区大小
>     int stackaddr_set; // 线程的栈设置
>     void *stackaddr; // 线程栈的位置
>     size_t stacksize; // 线程栈的大小
> } pthread_attr_t;
> ```
>
> 该结构体中成员变量的值不能直接修改，需要使用函数进行相关操作。在调用`pthread_create()`函数之前，初始化线程属性结构体的函数是`pthread_attr_init()`，它位于函数库`pthread.h`中，其函数原型如下：
>
> ```c
> int pthread_attr_init(pthread_attr_t *attr);
> ```
>
> pthread_attr_init()函数的参数attr是一个指向pthread_attr_t结构体类型的指针，它指向一个线程属性对象，用于设置线程的属性。如果线程属性对象attr初始化成功，则返回0，否则返回出错编号。
>
> 线程终止后需通过pthread_attr_destroy()函数销毁线程属性对象，该函数位于函数库pthread.h中，其函数原型如下：
>
> ```c
> int pthread_attr_destroy(pthread_attr_t *attr);
> ```
>
> pthread_attr_destroy()函数的参数attr是一个指向pthread_attr_t结构体类型的指针，它指向一个线程属性对象，用于销毁线程属性对象。如果线程属性对象attr销毁成功，则返回0，否则返回出错编号。
>
> 如若要是使用默认状态，将pthread_create()函数的参数attr设置为NULL即可。否则，需要使用系统中提供的函数接口来设置线程的属性。

#### 1.线程的分离状态

> 线程的分离状态决定了线程终止自身运行的方式，默认情况下线程处于非分离状态(PTHREAD_CREATE_JOINABLE)，系统中可以通过pthread_attr_setdetachstate()函数来设置线程属性中的分离状态，通过pthread_attr_getdetachstate()函数来获取线程属性中的分离状态。这两个函数位于函数库pthread.h中，其函数原型如下：
>
> ```c
> int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate);
> int pthread_attr_getdetachstate(pthread_attr_t *attr, int *detachstate);
> ```
>
> pthread_attr_setdetachstate()函数的第一个参数attr是一个指向pthread_attr_t结构体类型的指针，它指向一个线程属性对象，用于设置线程的属性。第二个参数detachstate是一个整型值，它指定了线程的分离状态，它可以是以下两个值之一：
> PTHREAD_CREATE_JOINABLE：线程处于非分离状态，这是线程的默认状态，这种情况下，线程需要被其他线程回收，否则会成为僵尸线程。
> PTHREAD_CREATE_DETACHED：线程处于分离状态，这种情况下，线程结束时会自动回收它的资源，而不需要其他线程调用pthread_join()函数来回收。
> 如果线程属性对象attr设置成功，则返回0，否则返回出错编号。
> pthread_attr_getdetachstate()函数的第一个参数attr是一个指向pthread_attr_t结构体类型的指针，它指向一个线程属性对象，用于获取线程的属性。第二个参数detachstate是一个整型指针，用于存储线程的分离状态。如果线程属性对象attr获取成功，则返回0，否则返回出错编号。

#### 线程分离状态的程序示例

```c
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
```

#### 2.线程的调度策略

> 线程的调度策略决定了线程在多个同优先级的线程中被调度的顺序。Linux支持三种线程调度策略，分别是SCHED_FIFO、SCHED_RR和SCHED_OTHER。默认情况下，线程的调度策略是SCHED_OTHER，系统中可以通过pthread_attr_setschedpolicy()函数来设置线程属性中的调度策略，通过pthread_attr_getschedpolicy()函数来获取线程属性中的调度策略。这两个函数位于函数库pthread.h中，其函数原型如下：
>
> ```c
> int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy);
> int pthread_attr_getschedpolicy(pthread_attr_t *attr, int *policy);
> ```
>
> pthread_attr_setschedpolicy()函数的第一个参数attr是一个指向pthread_attr_t结构体类型的指针，它指向一个线程属性对象，用于设置线程的属性。第二个参数policy是一个整型值，它指定了线程的调度策略，它可以是以下三个值之一：
>
> - SCHED_FIFO (First-In, First-Out):
这是一种实时调度策略，其中拥有最高优先级的线程总是首先被调度。如果有两个相同优先级的线程，它们将会按照它们到达就绪队列（即FIFO队列）的顺序来进行调度。在该策略下，一旦一个线程开始运行，它会继续运行直到它自己放弃CPU（例如，它阻塞在某些I/O操作上，或者主动调用了sched_yield()来让出CPU），或是被更高优先级的线程抢占。
>
> - SCHED_RR (Round-Robin):
这也是一种实时调度策略，它类似于SCHED_FIFO，但是为了防止相同优先级线程中的一个线程长时间占据CPU（“饿死”其他线程），它引入了时间片的概念。在SCHED_RR策略中，线程会运行一个时间片，然后被放回就绪队列的尾部，等待下一次调度。如果就绪队列中没有其他相同优先级的线程，则当前线程会立刻再次被调度。
>
> - SCHED_OTHER (Standard Round-Robin with Static Priority):
这是Linux默认的普通非实时调度策略。在这种策略下，每个线程都有一个nice值，该值可以影响线程的优先级。nice值越高，线程的优先级越低，反之亦然。SCHED_OTHER使用复杂的算法来动态调整线程的优先级，以确保公平的CPU时间分享。这种策略不适用于实时任务，但适用于大多数普通的时间共享场景。
>
> 如果线程属性对象attr设置成功，则返回0，否则返回出错编号。
> **设置实时调度策略通常需要超级用户权限（root权限）。**

#### 3.线程的调度参数

> 线程的调度参数是一个struct sched_param类型的结构体
>
>```c
> struct sched_param
> {
>     int sched_priority; // 线程的优先级
> };
>```
>
> 该结构体中包含一个整型成员变量sched_priority，它指定了线程的优先级。当线程的调度策略是实时调度策略时，可以通过pthread_attr_setschedparam()函数来设置线程属性中的调度参数，通过pthread_attr_getschedparam()函数来获取线程属性中的调度参数。这两个函数位于函数库pthread.h中，其函数原型如下：
>
> ```c
> int pthread_attr_setschedparam(pthread_attr_t *attr, const struct sched_param *param);
> int pthread_attr_getschedparam(pthread_attr_t *attr, struct sched_param *param);
> ```
>
> pthread_attr_setschedparam()函数的第一个参数attr是一个指向pthread_attr_t结构体类型的指针，它指向一个线程属性对象，用于设置线程的属性。第二个参数param是一个指向struct sched_param类型的指针，它指向一个线程的调度参数结构体，用于设置线程的调度参数。如果线程属性对象attr设置成功，则返回0，否则返回出错编号。

#### 4.线程的继承性

> 线程的继承性决定了线程是否继承创建它的线程的调度策略和调度参数。默认情况下，线程的继承性是PTHREAD_INHERIT_SCHED，系统中可以通过pthread_attr_setinheritsched()函数来设置线程属性中的继承性，通过pthread_attr_getinheritsched()函数来获取线程属性中的继承性。这两个函数位于函数库pthread.h中，其函数原型如下：
>
> ```c
> int pthread_attr_setinheritsched(pthread_attr_t *attr, int inheritsched);
> int pthread_attr_getinheritsched(pthread_attr_t *attr, int *inheritsched);
> ```
>
> pthread_attr_setinheritsched()函数的第一个参数attr是一个指向pthread_attr_t结构体类型的指针，它指向一个线程属性对象，用于设置线程的属性。第二个参数inheritsched是一个整型值，它指定了线程的继承性，它可以是以下两个值之一：
> PTHREAD_INHERIT_SCHED：线程继承创建它的线程的调度策略和调度参数。
> PTHREAD_EXPLICIT_SCHED：线程不继承创建它的线程的调度策略和调度参数。使用在attr属性对象中设置的调度策略和调度参数。
> 如果线程属性对象attr设置成功，则返回0，否则返回出错编号。

#### 线程调度策略和调度参数以及线程的继承性程序示例

```c
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
```

#### 5.线程的作用域

> 线程的作用域决定了线程的生命周期和线程对应的资源的可见性。线程的作用域可以是系统级（System Scope）或者进程级（Process Scope）。
>
> - 系统级线程：系统级线程在整个系统中可见，它们可以被调度到任何CPU上执行，具有更高的并行性，但也需要更多的系统资源。系统级线程通过clone系统调用创建。
>
> - 进程级线程：进程级线程在单个进程内可见，它们共享进程的地址空间和其他资源。进程级线程是通过pthread_create函数创建的标准POSIX线程。
> 默认情况下，线程的作用域是PTHREAD_SCOPE_SYSTEM，系统中可以通过pthread_attr_setscope()函数来设置线程属性中的作用域，通过pthread_attr_getscope()函数来获取线程属性中的作用域。这两个函数位于函数库pthread.h中，其函数原型如下：
>
> ```c
> int pthread_attr_setscope(pthread_attr_t *attr, int scope);
> int pthread_attr_getscope(pthread_attr_t *attr, int *scope);
> ```
>
> pthread_attr_setscope()函数的第一个参数attr是一个指向pthread_attr_t结构体类型的指针，它指向一个线程属性对象，用于设置线程的属性。第二个参数scope是一个整型值，它指定了线程的作用域，它可以是以下两个值之一：
> PTHREAD_SCOPE_SYSTEM：线程的作用域是系统级，这是线程的默认作用域。
> PTHREAD_SCOPE_PROCESS：线程的作用域是进程级。
> 如果线程属性对象attr设置成功，则返回0，否则返回出错编号。

#### 线程作用域的程序示例

```c
/*
在 POSIX 线程（pthreads）库中，线程的作用域（scope）可以决定线程是与进程中的其他线程（进程作用域，PTHREAD_SCOPE_PROCESS）竞争 CPU 时间，还是与系统中所有线程（系统作用域，PTHREAD_SCOPE_SYSTEM）竞争 CPU 时间。然而，并非所有操作系统都支持进程作用域的线程。

对于 Linux 系统，作用域通常是系统级的，因为 Linux 线程实现（基于 NPTL, Native POSIX Thread Library）通常将所有线程看作轻量级进程（LWP），它们与系统中的其他所有线程一样在内核调度器的控制下运行。

即使你尝试将线程的作用域设置为PTHREAD_SCOPE_PROCESS，这个设置可能会被忽略，因为 Linux 并不支持进程作用域的线程。这就解释了为什么即使你在pthread_attr_setscope中设置了作用域为PTHREAD_SCOPE_PROCESS，在子线程中打印出来的作用域还是PTHREAD_SCOPE_SYSTEM。

此外，pthread_attr_setscope调用可能返回成功（0），但实际上并没有改变作用域。你可以通过检查pthread_attr_setscope的返回值来确定设置是否成功。如果返回非零值，可能表示尝试设置一个不被系统支持的作用域。

在实际开发中，通常不需要改变线程的作用域，因为默认的系统作用域满足大多数应用程序的需要。如果确实需要对线程的调度行为进行更细粒度的控制，可以考虑使用线程的调度策略和优先级设置。
*/

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
```

#### 6.线程的栈

> Linux中用于修改和获取线程栈空间大小的函数为pthread_attr_setstacksize()和pthread_attr_getstacksize()，它们位于函数库pthread.h中，其函数原型如下：
>
> ```c
> int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize);
> int pthread_attr_getstacksize(pthread_attr_t *attr, size_t *stacksize);
> ```
>
> pthread_attr_setstacksize()函数的第一个参数attr是一个指向pthread_attr_t结构体类型的指针，它指向一个线程属性对象，用于设置线程的属性。第二个参数stacksize是一个size_t类型的值，它指定了线程栈的大小。如果线程属性对象attr设置成功，则返回0，否则返回出错编号。
>
> Linux中用于设置和获取栈地址的函数为pthread_attr_setstackaddr()和pthread_attr_getstackaddr()，它们位于函数库pthread.h中，其函数原型如下：
>
> ```c
> int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stackaddr);
> int pthread_attr_getstackaddr(pthread_attr_t *attr, void **stackaddr);
> ```
>
> pthread_attr_setstackaddr()函数的第一个参数attr是一个指向pthread_attr_t结构体类型的指针，它指向一个线程属性对象，用于设置线程的属性。第二个参数stackaddr是一个指向void类型的指针，它指向线程栈的位置。如果线程属性对象attr设置成功，则返回0，否则返回出错编号。
>
> Linux中用于设置和获取栈的警戒缓冲区大小的函数为pthread_attr_setguardsize()和pthread_attr_getguardsize()，它们位于函数库pthread.h中，其函数原型如下：
>
> ```c
> int pthread_attr_setguardsize(pthread_attr_t *attr, size_t guardsize);
> int pthread_attr_getguardsize(pthread_attr_t *attr, size_t *guardsize);
> ```
>
> pthread_attr_setguardsize()函数的第一个参数attr是一个指向pthread_attr_t结构体类型的指针，它指向一个线程属性对象，用于设置线程的属性。第二个参数guardsize是一个size_t类型的值，它指定了线程栈的警戒缓冲区大小。如果线程属性对象attr设置成功，则返回0，否则返回出错编号。
>
> Linux中可以调用一次就设置和获取线程属性对象中栈地址和栈容量的函数为pthread_attr_setstack()和pthread_attr_getstack()，它们位于函数库pthread.h中，其函数原型如下：
>
> ```c
> int pthread_attr_setstack(pthread_attr_t *attr, void *stackaddr, size_t stacksize);
> int pthread_attr_getstack(pthread_attr_t *attr, void **stackaddr, size_t *stacksize);
> ```
>
> pthread_attr_setstack()函数的第一个参数attr是一个指向pthread_attr_t结构体类型的指针，它指向一个线程属性对象，用于设置线程的属性。第二个参数stackaddr是一个指向void类型的指针，它指向线程栈的位置。第三个参数stacksize是一个size_t类型的值，它指定了线程栈的大小。如果线程属性对象attr设置成功，则返回0，否则返回出错编号。

## 线程同步

在 Linux 中，线程同步是指多个线程在执行过程中，通过一定机制协调它们的执行顺序，以确保它们对共享资源的访问是一致和协调的。线程同步主要用来避免竞态条件（race conditions），保证数据的一致性和完整性。

线程同步的概念出现是因为并发执行的线程往往需要访问和修改共享资源。如果对这些共享资源的访问不加以控制和协调，就可能导致不可预知的结果和程序错误。例如，如果两个线程同时修改同一个变量，最终变量的值将取决于线程执行的顺序和时机，这使得程序的行为变得不确定。

如果没有线程同步机制，多线程程序中可能会发生以下问题：

竞态条件（Race Conditions）：当多个线程同时修改同一个资源而没有适当的同步时，程序的输出可能取决于线程执行的顺序，导致不确定的结果。

数据不一致性：由于线程间的操作没有得到适当的协调，共享数据可能会处于不一致的状态。

死锁（Deadlocks）：线程可能会在等待某个不能被满足的条件（通常是等待其他线程释放资源）时无限阻塞。

活锁（Livelocks）和饥饿（Starvation）：线程可能因为总是让步给其他线程而无法取得进展，或者某些线程可能永远得不到执行的机会。

线程同步可以通过多种方式实现，包括：

互斥锁（Mutexes）：互斥锁允许只有一个线程在同一时间内持有锁，以此来访问共享资源。

读写锁（Read-Write Locks）：允许多个读取者同时访问资源，但写入者在写入时需要独占访问。

条件变量（Condition Variables）：线程可以在某种条件下挂起执行，直到另一个线程改变了这个条件。

信号量（Semaphores）：允许限制对资源访问的线程数。

屏障（Barriers）：用来同步线程群，使它们在某个点上同时停止或开始。

### 互斥锁

> 互斥锁（Mutex）是一种最常用的线程同步机制，它保证了多个线程在同一时间内互斥（Mutual Exclusive）地访问共享资源。互斥锁是一种二进制锁，它只有两种状态：锁定和未锁定。当一个线程获得了互斥锁后，其他线程就不能再获得该锁，直到锁被释放。如果其他线程试图获得该锁，那么它们将会被阻塞，直到互斥锁被释放。
> 使用互斥锁可以保证三点：
>
> - 原子性：如果在一个线程中设置了一个互斥锁，那么在加锁和解锁之间的操作会被锁定为一个原子操作，这些操作要么全部执行，要么全部不执行。
> - 唯一性：如果一个线程锁定了一个互斥锁，那么其他线程不能再锁定这个互斥锁，直到这个互斥锁被解锁。
> - 非繁忙等待：当一个线程尝试获取一个已经被另一个线程持有的互斥锁时，它不会在那里不断地检查锁是否被释放（这叫做自旋或者繁忙等待），相反，它会被挂起（进入睡眠状态），直到该锁被释放。一旦锁被释放，系统会自动唤醒在该锁上等待的线程之一。
>
> 使用互斥锁实现线程同步的一般步骤如下：
>
> 1. 定义互斥锁变量
> 2. 初始化互斥锁变量
> 3. 在需要同步的代码中加锁
> 4. 在需要同步的代码中解锁
> 5. 销毁互斥锁变量
> 在 Linux 中，互斥锁是通过`pthread_mutex_t`数据结构和一系列的宏和函数来实现的，例如`pthread_mutex_init`,`pthread_mutex_lock`,`pthread_mutex_unlock`, `pthread_mutex_destroy` 等。

#### `pthread_mutex_init()`函数

> `pthread_mutex_init()`函数用于初始化互斥锁变量，它位于函数库`pthread.h`中，其函数原型如下：
>
> ```c
> int pthread_mutex_init(pthread_mutex_t * restrict mutex, const pthread_mutexattr_t * restrict attr);
> ```
>
> - `mutex`：一个指向`pthread_mutex_t`结构体类型的指针，它指向一个互斥锁变量，用于初始化互斥锁。
> - `attr`：一个指向`pthread_mutexattr_t`结构体类型的指针，它指向一个互斥锁属性对象，用于设置互斥锁的属性。
> - `返回值`：互斥锁变量mutex初始化成功，则返回0，否则返回出错编号。
>
> 在 C 语言中，restrict 是一个函数参数修饰符，用于告诉编译器，通过被修饰的指针访问的内存空间，在其生命周期内，只能通过该指针本身或者该指针的拷贝来访问对应的内存空间。这个关键字只能用于指针类型，并且它是 C99 标准中引入的。当使用 restrict 关键字修饰指针时，程序员承诺该指针是访问对应内存空间的唯一途径，该内存空间不会被别的指针所访问。
> 基本上，restrict 关键字用于优化指针访问，提高代码性能。当你声明一个 restrict 修饰的指针时，你正在向编译器承诺：在该指针的作用域内，所指向的内存中的数据不会被其他独立的指针修改。这允许编译器进行更激进的优化，因为它可以假定两个 restrict 修饰的指针不会指向同一块内存区域。
> restrict 是一个单向约束，它仅对编译器有指导作用；它不会改变程序的行为，也不会在运行时强制执行，需要开发人员自觉遵守。
>
> **通过`pthread_mutex_init()`函数初始化互斥锁变量又称为动态初始化，此外互斥锁也可以直接使用宏`PTHREAD_MUTEX_INITIALIZER`进行静态初始化**，例如：
>
> ```c
> pthread_mutex_init(&mutex, NULL); // 动态初始化
> pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // 静态初始化
> ```

#### `pthread_mutex_lock()`函数

> `pthread_mutex_lock()`函数用于加锁互斥锁变量，它位于函数库`pthread.h`中，其函数原型如下：
>
> ```c
> int pthread_mutex_lock(pthread_mutex_t *mutex);
> ```
>
> - `mutex`：一个指向`pthread_mutex_t`结构体类型的指针，它指向一个互斥锁变量。
> - `返回值`：互斥锁变量mutex加锁成功，则返回0，否则返回出错编号。
>
> 若需要使用的互斥锁正在被使用，则调用`pthread_mutex_lock()`函数的线程将会被阻塞（被cpu挂起进入睡眠状态），有些情况下，希望线程先去执行其他功能，而不是一直等待互斥锁的解锁，这时可以使用非阻塞式的加锁函数`pthread_mutex_trylock()`函数，调用该函数的线程尝试加锁互斥量，若锁正在被使用，则线程不阻塞等待，直接返回错误码，它位于函数库`pthread.h`中，其函数原型如下：
>
> ```c
> int pthread_mutex_trylock(pthread_mutex_t *mutex);
> ```
>
> - `mutex`：一个指向`pthread_mutex_t`结构体类型的指针，它指向一个互斥锁变量。
> - `返回值`：互斥锁变量`mutex`加锁成功，则返回0，否则返回出错编号。

#### `pthread_mutex_unlock()`函数

> `pthread_mutex_unlock()`函数用于解锁互斥锁变量，它位于函数库`pthread.h`中，其函数原型如下：
>
> ```c
> int pthread_mutex_unlock(pthread_mutex_t *mutex);
> ```
>
> - `mutex`：一个指向`pthread_mutex_t`结构体类型的指针，它指向一个互斥锁变量。
> - `返回值`：互斥锁变量`mutex`解锁成功，则返回0，否则返回出错编号。

#### `pthread_mutex_destroy()`函数

> `pthred_mutex_destroy()`函数用于销毁互斥锁变量，它位于函数库`pthread.h`中，其函数原型如下：
>
> ```c
> int pthread_mutex_destroy(pthread_mutex_t *mutex);
> ```
>
> - `mutex`：一个指向pthread_mutex_t结构体类型的指针，它指向一个互斥锁变量。
> - `返回值`：互斥锁变量mutex销毁成功，则返回0，否则返回出错编号。

#### 互斥锁的程序示例

```c
// pthread_nomutex.c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

void* thread_func(void* arg)
{
    srand(time(NULL));
    while(1)
    {

        printf("%ld :hello", pthread_self());
        // 模拟长时间操作共享资源的并发环境，让系统调度其他线程上cpu执行
        sleep(rand()%3);
        printf("%ld :world\n", pthread_self());
        sleep(rand()%3);
    }
    pthread_exit(NULL);
}

int main()
{
    pthread_t mythread1, mythread2;     // 定义线程标识符

    srand(time(NULL));

    // 创建线程
    pthread_create(&mythread1, NULL, thread_func, NULL);
    pthread_create(&mythread2, NULL, thread_func, NULL);

    int i = 5;
    while(i--)
    {
        printf("%ld: HELLO ", pthread_self());
        sleep(rand()%3);
        printf("%ld: WORLD\n", pthread_self());
        sleep(rand()%3);
    }

    pthread_cancel(mythread1);
    pthread_cancel(mythread2);

    pthread_join(mythread1, NULL);
    pthread_join(mythread2, NULL);

    return 0;
}
```

```c
// pthread_mutex.c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>

pthread_mutex_t mutex; // 定义互斥锁变量

void* thread_func(void* arg)
{
    srand(time(NULL));
    while(1)
    {
        // 加锁互斥锁
        pthread_mutex_lock(&mutex);         // 加锁 mutex--
        printf("%ld :hello\n", pthread_self());
        // 模拟长时间操作共享资源的并发环境，让系统调度其他线程上cpu执行
        sleep(rand()%3);
        printf("%ld :world\n", pthread_self());
        // 解锁互斥锁
        pthread_mutex_unlock(&mutex);       // 解锁 mutex++
        sleep(rand()%3);
    }
    pthread_exit(NULL);
}

int main()
{
    pthread_t mythread1, mythread2;     // 定义线程标识符
    srand(time(NULL));

    // 初始化互斥锁变量
    pthread_mutex_init(&mutex, NULL);   // mutex = 1

    // 创建线程
    pthread_create(&mythread1, NULL, thread_func, NULL);
    pthread_create(&mythread2, NULL, thread_func, NULL);

    int i = 5;
    while(i--)
    {
        // 加锁互斥锁
        pthread_mutex_lock(&mutex);         // 加锁 mutex--
        printf("%ld: HELLO\n", pthread_self());
        sleep(rand()%3);
        printf("%ld: WORLD\n", pthread_self());
        // 解锁互斥锁
        pthread_mutex_unlock(&mutex);       // 解锁 mutex++
        sleep(rand()%3);
    }

    pthread_cancel(mythread1);
    pthread_cancel(mythread2);

    pthread_join(mythread1, NULL);
    pthread_join(mythread2, NULL);

    // 销毁互斥锁变量
    pthread_mutex_destroy(&mutex);

    return 0;
}
```

了解：在 Linux 中，自旋锁（spinlock）是一种用于多线程同步的低级原语，它在内核开发中非常常见。自旋锁的核心思想是线程在尝试获取锁时，如果发现锁已经被其他线程占用，它将循环等待（即“自旋”），不断检查锁的状态，直到锁变得可用。

自旋锁与互斥锁（mutex）的主要区别在于自旋锁的等待策略。当互斥锁不可用时，线程会进入睡眠状态，等待操作系统将它唤醒。相比之下，自旋锁不会使线程睡眠，而是使线程在一个紧凑的循环中持续检查锁的状态，这是一种繁忙等待（busy-waiting）。

自旋锁在以下情况下是有用的：

锁持有时间很短： 当线程只需要持有锁非常短的时间时，自旋锁的开销可能比阻塞和唤醒线程的开销要小。

上下文切换代价高： 在多处理器系统中，如果一个线程因为等待锁而睡眠，它的上下文（例如，寄存器状态、内存映射等）需要被保存和恢复，这会带来一定的开销。如果锁很快就会变得可用，那么使用自旋锁来避免这种上下文切换会更有效率。

不能睡眠的环境： 在某些环境中，如中断处理程序或其他不能阻塞的上下文中，自旋锁是控制并发的合适选择。

尽管自旋锁在某些情境下很有用，但在其他情况下可能会导致性能问题。如果线程长时间持有自旋锁，或者系统中有许多线程竞争同一个锁，自旋锁可能会导致大量的 CPU 时间浪费在无用的自旋上，而不是执行有用的工作。

在 Linux 内核中，自旋锁是通过 spinlock_t 数据结构和一系列的宏和函数来实现的，例如 spin_lock, spin_unlock, spin_lock_irqsave, spin_lock_irqrestore 等。

使用自旋锁时，开发者需要非常小心地评估其对性能的影响，并确保锁的使用逻辑正确，以避免死锁和竞态条件。

在Linux系统中，读写锁（Read-Write Locks），也称为共享-独占锁（Shared-Exclusive Locks），是同步机制的一种，用于控制对共享资源的并发访问。读写锁特别适用于那些读操作远多于写操作的场景，因为它们允许多个线程同时读取共享资源，但在任何给定时间内只允许一个线程写入。这样可以提高并发性，因为读操作通常不会修改资源，因此允许多个读操作同时进行通常是安全的。

读写锁通常通过以下几个操作来管理：

1. **读锁定（Shared Lock）**：
   - 当一个线程想要读取共享资源时，它会请求一个读锁定。
   - 如果没有线程持有写锁定，该线程将获得读锁定，可以进行读取操作。
   - 如果其他线程已经持有读锁定，当前请求的线程也可以获得读锁定，并且可以同时发生多个读操作。
   - 但是，如果有线程持有写锁定或等待写锁定，新的读锁定请求可能会被阻塞直到写锁定释放。

2. **写锁定（Exclusive Lock）**：
   - 当一个线程想要写入共享资源时，它会请求一个写锁定。
   - 如果没有其他线程持有读锁定或写锁定，该线程将获得写锁定，可以进行写入操作。
   - 如果已有线程持有读锁定或写锁定，写锁定请求会被阻塞直到所有其他锁定都被释放。
   - 在一个线程持有写锁定期间，任何其他的读锁定或写锁定请求都会被阻塞。

3. **解锁（Unlock）**：
   - 当一个线程完成其读取或写入操作后，它需要释放它持有的锁定，这样其他线程才能获取锁定并访问资源。

在Linux中，读写锁可以通过多种方式实现，包括使用POSIX线程库（pthread）中的`pthread_rwlock_t`类型和相应的函数来创建和管理读写锁。例如，`pthread_rwlock_rdlock`用于获取读锁定，`pthread_rwlock_wrlock`用于获取写锁定，而`pthread_rwlock_unlock`用于释放任一类型的锁定。

### 条件变量

使用条件变量（Condition Variables）可以实现线程间的通信，它可以让线程在满足特定条件时才执行。条件变量是一种同步机制，它允许线程在满足特定条件时才继续执行，否则线程将被阻塞。条件变量通常与互斥锁一起使用，以免出现竞态条件（Race Conditions）。

条件变量的使用一般步骤如下：

1. 定义条件变量
2. 初始化条件变量
3. 在需要同步的代码中加锁
4. 在需要同步的代码中等待条件变量
5. 在需要同步的代码中解锁
6. 在需要同步的代码中唤醒等待条件变量的线程
7. 销毁条件变量
8. 销毁互斥锁变量

针对上述步骤，Linux提供了一系列的函数来实现条件变量，例如`pthread_cond_init`, `pthread_cond_wait`,`pthread_cond_signal`, `pthread_cond_destroy`等。

#### `pthread_cond_init()`函数
  
> `pthread_cond_init()`函数用于初始化条件变量，它位于函数库`pthread.h`中，其函数原型如下：
>
> ```c
> int pthread_cond_init(pthread_cond_t * restrict cond, const pthread_condattr_t * restrict attr);
> ```
>
> pthread_cond_init()函数的第一个参数cond是一个指向pthread_cond_t结构体类型的指针，它指向一个条件变量，用于初始化条件变量。第二个参数attr是一个指向pthread_condattr_t结构体类型的指针，它指向一个条件变量属性对象，用于设置条件变量的属性。如果条件变量cond初始化成功，则返回0，否则返回出错编号。
>
> pthread_condattr_t *attr参数可以为NULL，表示使用默认的条件变量PTHREAD_PROCESS_PRIVATE属性，表示条件变量只能在当前进程内使用。如果需要在多个进程间使用条件变量，则需要设置条件变量的属性为PTHREAD_PROCESS_SHARED
>
> 除使用pthread_cond_init()函数初始化条件变量外，还可以直接使用宏PTHREAD_COND_INITIALIZER进行静态初始化，例如：
>
> ```c
> pthread_cond_init(&cond, NULL);
> pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
> ```
>
> pthread_cond_wait()函数用于等待条件变量，它位于函数库pthread.h中，其函数原型如下：
>
> ```c
> int pthread_cond_wait(pthread_cond_t * restrict cond, pthread_mutex_t * restrict mutex);
> ```
>
> pthread_cond_wait()函数的第一个参数cond是一个指向pthread_cond_t结构体类型的指针，它指向一个条件变量。第二个参数mutex是一个指向pthread_mutex_t结构体类型的指针，它指向一个互斥锁变量。如果条件变量cond等待成功，则返回0，否则返回出错编号。
>
> 在Linux中，`pthread_cond_wait()` 函数是 POSIX 线程（pthread）库提供的一个条件变量等待函数。条件变量提供了一种线程同步机制，用来在某些条件下阻塞线程，直到某个特定条件为真为止。

这是 `pthread_cond_wait()` 函数的基本作用：当线程调用这个函数时，它会被阻塞，直到另一个线程通过相同条件变量调用 `pthread_cond_signal()` 或 `pthread_cond_broadcast()` 来唤醒它。

执行流程和工作机制大致如下：

1. **等待条件变量**：
   在调用 `pthread_cond_wait()` 之前，线程已经获取了相关联的互斥锁（mutex），该互斥锁用来保护条件相关的共享资源。调用 `pthread_cond_wait()` 意味着线程等待某个条件成为真。

2. **释放互斥锁并阻塞**：
   当线程调用 `pthread_cond_wait()` 时，函数将执行两个步骤：首先，释放已经获取的互斥锁，这样其他线程就可以获取这个锁来改变条件或继续工作；其次，阻塞调用线程直到条件变量被信号唤醒。

3. **等待信号**：
   线程在调用 `pthread_cond_wait()` 后会一直阻塞，直到其他线程调用 `pthread_cond_signal()` 或 `pthread_cond_broadcast()` 对同一个条件变量发出信号。`pthread_cond_signal()` 唤醒等待同一条件变量的一个线程，而 `pthread_cond_broadcast()` 唤醒所有等待的线程。

4. **重新获取互斥锁**：
   当线程被唤醒后，它会尝试重新获取与条件变量关联的互斥锁。只有当它再次获取到互斥锁后，线程才会从 `pthread_cond_wait()` 函数返回。

5. **检查条件**：
   `pthread_cond_wait()` 返回后，线程通常需要手动重新检查条件是否真的满足了，因为有可能发生所谓的“虚假唤醒”，或者多个线程被同时唤醒，但只能有一个线程继续执行。

这个机制允许线程以无竞争的方式等待条件变化，因为当条件变量不满足时，线程不会占用 CPU 资源，它们会被挂起直到条件变化为止。这样做可以提高程序的效率，特别是在那些需要等待特定事件或条件的应用程序中。

### 通过一个示例先理解一下条件变量中的wait和signal

```c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// 定义一个全局变量，表明缓冲区中项的状态，0表示缓冲区空，1表示缓冲区不空
int buffer_has_item = 0;
pthread_mutex_t mutex;  // 定义互斥锁变量
pthread_cond_t cond;    // 定义条件变量     这个条件变量是和buffer_has_item挂钩的

// 消费者函数
void* consumer(void *arg) {
    /*
    循环去执行消费者的任务，消费者线程和生产者线程是并行的，且消费者线程和生产者线程使用同一个互斥锁和条件变量
    */
    while (1) {
        pthread_mutex_lock(&mutex); // 如果消费者线程成功加锁，则生产者线程就会阻塞，如果有多个消费者线程，则其他消费者线程会阻塞
        while (buffer_has_item == 0) { // 循环等待条件变量，如果缓冲区为空，则消费者线程就会执行下面的wait函数进入阻塞，即不满足条件变量
            pthread_cond_wait(&cond, &mutex); // 等待条件变量，同时释放互斥锁，使得生产者线程可以有机会获得互斥锁，阻塞的线程被唤醒（说明收到信号表明此时满足条件变量）但不表示 pthread_cond_wait函数会返回，只有再次拿到互斥锁后pthread_cond_wait函数才会返回，这里就会存在消费者线程被唤醒但是此时生产者线程还没有释放互斥锁，或者互斥锁被释放后多个线程（包括消费者线程、生产者线程）只有一个能够抢到互斥锁。此时pthread_cond_wait返回的消费者线程会再次进入循环判断是否满足条件（是否满足条件变量），这里再次判断是否满足条件变量的原因是如果有多个消费者线程被唤醒，假设消费者线程a在获得互斥锁之前（pthread_cond_wait函数还没返回），消费者线程b获得互斥锁（pthread_cond_wait函数返回）——>修改条件变量——>释放互斥锁，那么线程a在拿到互斥锁（pthread_cond_wait函数返回），但是条件变量不满足，就会再次被阻塞持续上面的步骤。
        }
        // 执行到这里说明满足条件变量，消费项...
        printf("消费者消费了一个项\n");
        buffer_has_item = 0; // 消费完后设置缓冲区为空
        pthread_mutex_unlock(&mutex);
        sleep(1); // 模拟消费者消费商品需要的时间
    }
    return NULL;
}

// 生产者函数
void* producer(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex); // 如果生产者线程成功加锁，则消费者线程就会阻塞，如果有多个生产者线程，则其他生产者线程会阻塞
        if (buffer_has_item == 0) { // 如果缓冲区为空，则生产者线程就会生产商品，如果缓冲区不为空，则直接释放互斥锁
            // 生产项...
            printf("生产者生产了一个项\n");
            buffer_has_item = 1; // 生产完后设置缓冲区不为空
            pthread_cond_signal(&cond); // 发送条件变量信号，唤醒因为条件变量不满足而阻塞的一个消费者线程，如果有多个消费者线程因为条件变量不满足而阻塞，唤醒哪个消费者线程有调度策略决定
            // pthread_cond_broadcast(&cond); // 如果有多个消费者线程因为条件变量不满足而阻塞，生产者线程广播条件变量信号，唤醒所有因为条件变量不满足而阻塞的消费者线程
        }
        pthread_mutex_unlock(&mutex);
        sleep(2); // 模拟生产者生产商品需要的时间
    }
    return NULL;
}

int main() {
    pthread_t prod, cons;

    // 初始化互斥锁和条件变量
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    // 创建生产者和消费者线程
    pthread_create(&prod, NULL, producer, NULL);
    pthread_create(&cons, NULL, consumer, NULL);

    // 等待线程结束
    pthread_join(prod, NULL);
    pthread_join(cons, NULL);

    // 销毁互斥锁和条件变量
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}
```

> 除了pthread_cond_wait()函数外，还有一个pthread_cond_timedwait()函数，它的作用和pthread_cond_wait()函数类似，但是它可以设置一个超时时间，如果超过了这个时间，线程就会被唤醒，它位于函数库pthread.h中，其函数原型如下：
>
> ```c
> int pthread_cond_timedwait(pthread_cond_t * restrict cond, pthread_mutex_t * restrict mutex, const struct timespec * restrict abstime);
> ```
>
> pthread_cond_timedwait()函数的第一个参数cond是一个指向pthread_cond_t结构体类型的指针，它指向一个条件变量。第二个参数mutex是一个指向pthread_mutex_t结构体类型的指针，它指向一个互斥锁变量。第三个参数abstime是一个指向struct timespec结构体类型的指针，它指向一个绝对时间，即从1970年1月1日0时0分0秒到指定时间的秒数。struct timespec结构体类型定义如下：
>
> ```c
> struct timespec {
>    time_t tv_sec; // 秒
>    long tv_nsec;  // 纳秒
> };
> ```
>
> 如果条件变量cond等待成功，则返回0，否则返回出错编号。
>
> pthread_cond_signal()函数用于唤醒等待条件变量的线程，它位于函数库pthread.h中，其函数原型如下：
>
> ```c
> int pthread_cond_signal(pthread_cond_t *cond);
> ```
>
> pthread_cond_signal()函数的参数cond是一个指向pthread_cond_t结构体类型的指针，它指向一个条件变量。如果条件变量cond唤醒成功，则返回0，否则返回出错编号。
>
> pthread_cond_broadcast()函数用于广播唤醒等待条件变量的线程，它位于函数库pthread.h中，其函数原型如下：
>
> ```c
> int pthread_cond_broadcast(pthread_cond_t *cond);
> ```
>
> pthread_cond_broadcast()函数的参数cond是一个指向pthread_cond_t结构体类型的指针，它指向一个条件变量。如果条件变量cond广播唤醒成功，则返回0，否则返回出错编号。
>
> pthread_cond_destroy()函数用于销毁条件变量，它位于函数库pthread.h中，其函数原型如下：
>
> ```c
> int pthread_cond_destroy(pthread_cond_t *cond);
> ```
>
> pthread_cond_destroy()函数的参数cond是一个指向pthread_cond_t结构体类型的指针，它指向一个条件变量。如果条件变量cond销毁成功，则返回0，否则返回出错编号。
> 需要注意的是，只有在没有线程等待条件变量时，才能销毁条件变量，否则该函数会返回EBUSY错误。
>

### 生产者-消费者模型程序示例

```c
// pthread_cond.c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // 定义并初始化互斥锁变量
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;    // 定义并初始化条件变量

struct product
{
    int proId;      // 产品编号
    struct product *next;   // 指向下一个产品的指针   
};

struct product *head = NULL;    // 定义头指针，默认初始化为NULL

int curNum = 0;     // 当前产品数量，默认初始化为0

// 消费者函数
void* consumer(void *arg)
{
    while(1)
    {
        pthread_mutex_lock(&mutex); // 如果消费者线程成功加锁，则生产者线程就会阻塞，如果有多个消费者线程，则其他消费者线程会阻塞
        while(curNum == 0) // 循环等待条件变量，如果缓冲区为空，则消费者线程就会执行下面的wait函数进入阻塞，即不满足条件变量
        {
            pthread_cond_wait(&cond, &mutex);
        
        }
        // 执行到这里说明满足条件变量，消费项...
        struct product *pro = head;
        head = head->next;
        printf("消费者消费了一个产品，产品编号为：%d， 当前产品剩余数量为：%d\n", pro->proId, --curNum);
        free(pro);
        pthread_mutex_unlock(&mutex);
        sleep(rand()%3); // 模拟消费者消费商品需要的时间
    }
    return NULL;
}

// 生产者函数
void* producer(void *arg)
{
    while(1)
    {
        pthread_mutex_lock(&mutex); // 如果生产者线程成功加锁，则消费者线程就会阻塞，如果有多个生产者线程，则其他生产者线程会阻塞
        struct product *pro = (struct product*)malloc(sizeof(struct product));
        pro->proId = rand()%1000 + 1;
        curNum++;
        printf("生产者生产了一个产品，产品编号为：%d， 当前产品剩余数量为：%d\n", pro->proId, curNum);
        /*
        产品是用链表存储的，生产者不用考虑产品的数量，只要生产产品就将产品放到链表中，如果要限制产品的数量，可以定义个宏，当链表中的产品数量达到宏定义的数量时，生产者就不再生产产品，直到消费者消费了产品后，生产者才继续生产产品
        */
        pro->next = head;
        head = pro;
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&cond); // 发送条件变量信号，唤醒因为条件变量不满足而阻塞的一个消费者线程，如果有多个消费者线程因为条件变量不满足而阻塞，唤醒哪个消费者线程有调度策略决定
        // 模拟生产者生产商品需要的时间
        sleep(rand()%5);
    }
    return NULL;
}

int main()
{
    pthread_t prod1, prod2, cons1, cons2, cons3;

    // 创建生产者和消费者线程
    pthread_create(&prod1, NULL, producer, NULL);
    pthread_create(&prod2, NULL, producer, NULL);
    pthread_create(&cons1, NULL, consumer, NULL);
    pthread_create(&cons2, NULL, consumer, NULL);
    pthread_create(&cons3, NULL, consumer, NULL);

    // 等待线程结束
    pthread_join(prod1, NULL);
    pthread_join(prod2, NULL);
    pthread_join(cons1, NULL);
    pthread_join(cons2, NULL);
    pthread_join(cons3, NULL);

    // 销毁互斥锁和条件变量
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}
```

## 信号量

若有线程申请访问共享资源，系统会执行P操作使共享资源的信号量减1，当信号量的值小于0时，线程会被阻塞，直到信号量的值大于0；若有线程释放共享资源，系统会执行V操作使信号量加1，当信号量的值大于0时，线程可以访问共享资源。

### `sem_init()`函数

> `sem_init()`函数用于初始化信号量，它位于函数库`semaphore.h`中，其函数原型如下：
>
> ```c
> int sem_init(sem_t *sem, int pshared, unsigned int value);
> ```
>
> - `sem`：一个指向`sem_t`结构体类型的指针，它指向一个信号量。
> - `pshared`：用于控制信号量的作用范围，它的取值如下：
>   - `0`：信号量将会被放在进程中所有线程可见的地址内，由进程中的线程共享。
>   - `非0`：信号量将会被放置在共享内存区域，由所有进程共享。
> - `value`：用于指定信号量的初始值。
> - `返回值`：信号量初始化成功，则返回0，否则返回-1并设置出错编号。

### `sem_wait()`函数

> `sem_wait()`函数用于等待信号量（对应P操作），它位于函数库`semaphore.h`中，其函数原型如下：
>
> ```c
> int sem_wait(sem_t *sem);
> ```
>
> - `sem`：一个指向`sem_t`结构体类型的指针，它指向一个信号量。
> - `返回值`：若函数执行成功，则会使信号量`sem`的值减 1 并返回0，若调用失败，则返回-1并设置出错编号。
> - `注意`：如果信号量的值大于0，则将信号量的值减1，然后返回；如果信号量的值等于0，则线程阻塞等待，直到信号量的值大于0，然后将信号量的值减1，然后返回。若不希望线程阻塞等待，可以使用非阻塞式的等待函数`sem_trywait()`函数，它位于函数库`semaphore.h`中，其函数原型如下：
>
> ```c
> int sem_trywait(sem_t *sem);
> ```

### `sem_post()`函数

> `sem_post()`函数用于发送信号量（对应V操作），它位于函数库`semaphore.h`中，其函数原型如下：
>
> ```c
> int sem_post(sem_t *sem);
> ```
>
> - `sem`：一个指向`sem_t`结构体类型的指针，它指向一个信号量。
> - `返回值`：若函数执行成功，则会使信号量`sem`的值加 1 并返回0，若调用失败，则返回-1并设置出错编号。

### `sem_destroy()`函数

> `sem_destroy()`函数用于销毁信号量，它位于函数库`semaphore.h`中，其函数原型如下：
>
> ```c
> int sem_destroy(sem_t *sem);
> ```
>
> - `sem`：一个指向`sem_t`结构体类型的指针，它指向一个信号量。
> - `返回值`：若函数调用成功，则返回0，若调用失败，则返回-1并设置出错编号。

### 其他信号量函数

> 除了上述信号量函数外，Linux还提供了一些其他的信号量函数，例如：
>
> - `sem_getvalue()`：获取信号量的值。
> - `sem_timedwait()`：设置信号量的超时时间。
> - `sem_unlink()`：删除一个命名的信号量。
> - `sem_open()`：打开一个命名的信号量。
> - `sem_close()`：关闭一个命名的信号量。

### 信号量的程序示例

```c
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM 5

int queue[NUM]; // 定义一个数组作为队列

sem_t blank_number, product_number; // 定义两个信号量，分别表示缓冲区中空闲空间的数量和产品的数量

void *producer_func(void *arg)
{
    int i = 0;
    while(1)
    {
        sem_wait(&blank_number); // 如果信号量blank_number的值大于0，则将信号量的值减1，然后返回；如果信号量的值等于0，则线程阻塞等待，直到信号量的值大于0，然后将信号量的值减1，然后返回
        queue[i] = rand()%1000 + 1; // 生产一个产品
        printf("生产者生产了一个产品，产品编号为：%d\n", queue[i]);
        sem_post(&product_number); // 将信号量product_number的值加1
        i = (i+1)%NUM;
        sleep(rand()%3);
    }
}

void *consumer_func(void *arg)
{
    int i = 0;
    while(1)
    {
        sem_wait(&product_number); // 如果信号量product_number的值大于0，则将信号量的值减1，然后返回；如果信号量的值等于0，则线程阻塞等待，直到信号量的值大于0，然后将信号量的值减1，然后返回
        printf("消费者消费了一个产品，产品编号为：%d\n", queue[i]);
        queue[i] = 0; // 消费一个产品
        sem_post(&blank_number); // 将信号量blank_number的值加1
        i = (i+1)%NUM;
        sleep(rand()%3);
    }
}


int main(int argc, char **argv)
{
    pthread_t producer, consumer; // 定义两个线程标识符，分别表示生产者线程和消费者线程

    sem_init(&blank_number, 0, NUM); // 初始化信号量blank_number为NUM
    sem_init(&product_number, 0, 0); // 初始化信号量product_number为0

    pthread_create(&producer, NULL, producer_func, NULL); // 创建生产者线程
    pthread_create(&consumer, NULL, consumer_func, NULL); // 创建消费者线程
    pthread_create(&consumer, NULL, consumer_func, NULL); // 创建消费者线程

    pthread_join(producer, NULL); // 等待生产者线程结束
    pthread_join(consumer, NULL); // 等待消费者线程结束

    sem_destroy(&blank_number); // 销毁信号量blank_number
    sem_destroy(&product_number); // 销毁信号量product_number

    return 0;
}
```
