# 文件操作

在 Linux 中，每个进程都有一个打开文件描述符表（open file description table），它是一个用于跟踪进程打开的文件和其他 I/O 资源的数据结构。这个表中包含了指向进程正在使用的文件、套接字、管道等资源的引用，允许进程对这些资源进行读取、写入和其他操作。

打开文件描述符表是一个数组，索引是文件描述符（file descriptor），它是非负整数。标准输入(STDIN_FILENO)、标准输出(STDOUT_FILENO)和标准错误(STDERR_FILENO)的文件描述符分别是 0、1 和 2。当进程打开一个新的文件或者建立一个新的 I/O 连接时，内核会分配一个未使用的文件描述符，并将其分配给新打开的文件或者连接。这个文件描述符在进程内部用于标识和访问相应的资源。

进程的打开文件描述符表是进程上下文的一部分，每个进程都有自己独立的表。这意味着一个进程不能直接访问另一个进程的打开文件描述符。打开文件描述符表在进程执行的整个生命周期中都存在，并且会在进程终止时自动释放。

打开文件描述符表对于进程来说非常重要，因为它允许进程并发地访问多个文件和 I/O 资源，从而实现了对这些资源的统一管理和操作。

系统级的文件描述符表，也称为打开文件表（open file table），是在操作系统内核中用于管理系统中所有打开文件和其他 I/O 资源的数据结构。每个打开的文件在系统级的文件描述符表中都有一个条目被称为打开文件句柄（open file handle），这些条目记录了文件的状态、位置、权限等信息，并维护了对应的文件对象。

系统级的文件描述符表通常由操作系统内核维护，它与用户级的文件描述符表（进程级）不同，后者用于跟踪进程自身打开的文件和 I/O 资源。系统级的文件描述符表是全局的，对系统中所有进程和线程都是可见的并且被所有进程和线程所共享，每个进程通过其用户级的文件描述符表与系统级的文件描述符表进行交互。

系统级的文件描述符表为操作系统提供了一个集中管理所有打开文件的机制，允许操作系统在内核级别对文件进行统一管理和操作。它包含了系统中所有打开的文件和 I/O 资源的引用，可以用于进行文件的读写、定位、关闭等操作，并且可以跟踪文件的状态变化。

系统级的文件描述符表还包含了与每个打开文件相关的文件对象，文件对象中包含了文件的元数据、位置、引用计数等信息，以及指向文件系统相关数据结构的指针，这些数据结构用于实际的文件 I/O 操作。

## 文件I/O（无缓存）

### `open()`函数

> `open()`函数的功能用于打开或创建一个文件，该函数存在于系统函数库`fcntl.h`中，其函数原型为：
>
> ```c
> int open(const char *pathname, int flags, mode_t mode);
> ```
>
> - `pathname`：文件路径名
> - `flags`：打开文件的方式，常用`fcntl.h`中的宏定义
> - `mode`：文件的权限，**只有当第二个参数flags为`O_CREAT`时才有效**
> - 返回值：成功返回文件描述符，失败返回-1

`flags`参数的取值如下：
| 编号 | 宏 | 说明 |
|:---:|:-------:|:--------:|
| 1 | O_RDONLY | 只读打开 |
| 2 | O_WRONLY | 只写打开 |
| 3 | O_RDWR   | 读写打开 |
| - | -------- | 1-3必选其一 |
| 4 | O_APPEND | 追加打开 |
| 5 | O_CREAT  | 文件不存在则创建 |
| 6 | O_EXCL   | 与O_CREAT一起使用，文件存在则报错 |
| 7 | O_TRUNC  | 文件以只写或读写打开时将其长度截断为0 |
| 8 | O_NOCTTY | 如果pathname指向终端设备，不要把这个设备分配给进程用作控制终端 |
| - | -------- | 4-8可选通过 \| 符合添加 |
| 9 | O_NONBLOCK | 非阻塞方式打开 |
| 10 | O_SYNC | 等待物理I/O操作完成后再write或者read，用于同步I/O |
| 11 | O_DSYNC | 等待物理I/O操作完成后再write，用于同步写 |
| 12 | O_RSYNC | 等待物理I/O操作完成后再read，用于同步读 |

可以直接使用`creat()`函数创建一个新文件，其函数原型为：

```c
int creat(const char *pathname, mode_t mode);
```

### `read()`函数

> `read()`函数用于从已打开的设备或文件中读取数据，该函数存在于系统函数库`unistd.h`中，其函数原型为：
>
> ```c
> ssize_t read(int fd, void *buf, size_t count);
> ```
>
> - `fd`：文件描述符
> - `buf`：读取数据的缓冲区
> - `count`：读取的字节数
> - 返回值：成功返回读取的字节数，失败返回-1

```c
// 如果想读取二进制数据，可以使用下面的方式
unsigned char buffer[100]; // 缓冲区定义为 unsigned char* 类型
int bytes_read = read(fd, buffer, sizeof(buffer));
```

### `write()`函数

> `write()`函数用于向已打开的设备或文件中写入数据，该函数存在于系统函数库`unistd.h`中，其函数原型为：
>
> ```c
> ssize_t write(int fd, const void *buf, size_t count);
> ```
>
> - `fd`：文件描述符
> - `buf`：写入数据的缓冲区
> - `count`：写入的字节数
> - 返回值：成功返回写入的字节数，失败返回-1并设置errno

### `lseek()`函数

> 每个打开的文件都有一个当前文件偏移量，该数值是一个非负整数，表示当前文件的读写位置
> `lseek()`函数用于移动文件读写指针，该函数存在于系统函数库`unistd.h`中，其函数原型为：
>
> ```c
> off_t lseek(int fd, off_t offset, int whence);
> ```
>
> - `fd`：文件描述符
> - `offset`：用于对文件偏移量的设置，该参数的值可以是正数、负数或0
> - `whence`：用于指定文件偏移量的起始位置，该参数的值可以是下列常量：
>   - `SEEK_SET`：从文件头开始偏移
>   - `SEEK_CUR`：从当前读写位置开始偏移
>   - `SEEK_END`：从文件尾开始偏移
> - 返回值：成功返回新的读写指针位置，失败返回-1

```c
// 使用 lseek 将文件指针定位到文件末尾
offset = lseek(fd, 0, SEEK_END);

// 将文件指针设置回文件开头
offset = lseek(fd, 0, SEEK_SET);
```

### `close()`函数

> `close()`函数用于关闭一个已打开的文件，该函数存在于系统函数库`unistd.h`中，其函数原型为：
>
> ```c
> int close(int fd);
> ```
>
> - `fd`：文件描述符
> - 返回值：成功返回0，失败返回-1

#### 关于以上几个函数的示例

```c
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define BUFFER_SIZE 1024    // define buffer size

typedef void (*err_handle)(const char*); // define error handle function

void err_exit(const char *err_msg)
{
    perror(err_msg);
    exit(EXIT_FAILURE);
}

int main()
{
    // file path
    const char *filePath = "/home/philo/professional/CSCourse/LinuxPractice/FileIO/1.txt";

    // content
    const char *content = "Hello World! Hello Philo";

    
    int fd; // file descriptor 
    // open file
    /*
    O_CLOEXEC表示在程序执行exec函数时，会关闭文件描述符（file descriptor）。
    假设我们有一个父进程和一个子进程，父进程打开了一个文件，然后使用fork创建了一个子进程。如果在打开文件时没有使用O_CLOEXEC选项，那么子进程会继承父进程打开的文件描述符。当父子进程中的子进程执行exec函数时，如果没有显式关闭文件描述符，那么该文件描述符将一直保持打开状态，直到进程退出或显式关闭文件描述符。
    如果在打开文件时使用了O_CLOEXEC选项，那么在子进程执行exec函数时，文件描述符会被自动关闭，不会一直保持打开状态，这有助于避免资源泄漏和提高程序的安全性。
    */
    fd = open(filePath, O_RDWR | O_CLOEXEC | O_CREAT | O_TRUNC, 0777);
    if(fd == -1)
    {
        fprintf(stderr, "Failed to open file: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Open file successfully\n");
    }

    // write content to file
    ssize_t bytes_write; // number of bytes written
    bytes_write = write(fd, content, strlen(content));
    if(bytes_write == -1)
    {
        fprintf(stderr, "Failed to write to file: %s\n", strerror(errno));
        close(fd);
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Write content to file successfully\n");
    }
    
    char buffer[BUFFER_SIZE]; // def buffer to read content from file
    ssize_t bytes_read; // number of bytes read
    bytes_read = read(fd, buffer, BUFFER_SIZE); // sizeof(buffer)
    if(bytes_read == -1)
    {
        fprintf(stderr, "Failed to read from file: %s\n", strerror(errno));
        close(fd);
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Read content from file successfully\n");
    }

    // print content to screen
    printf("Content of file: %s\n", buffer);
    // write(STDOUT_FILENO, buffer, numRead);

    //  
    if(lseek(fd, 0, SEEK_SET) == -1)
    {
        fprintf(stderr, "Failed to lseek: %s\n", strerror(errno));
        close(fd);
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("lseek successfully\n");
    }

    // file path
    const char *newFilePath = "/home/philo/professional/CSCourse/LinuxPractice/FileIO/2.txt";

    // open file
    int new_fd; 
    new_fd = open(newFilePath, O_RDWR | O_CREAT | O_TRUNC, 0777);
    if(new_fd == -1)
    {
        perror("Failed to open new file");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Open new file successfully\n");
    }

    // copy content from file to new file
    while(bytes_read = read(fd, buffer, BUFFER_SIZE) > 0)
    {
        if(write(new_fd, buffer, bytes_read) == -1)
        {
           perror("Failed to write to file");
           close(new_fd);
           close(fd);
           exit(EXIT_FAILURE);
        }

    }

    // close file
    close(new_fd);
    close(fd);

    char *flag = "FILE CONTENT copied successfully";
    write(STDOUT_FILENO, flag, strlen(flag));

    return 0;
}
```

### `dup()`函数

> `dup()`函数用于复制一个已打开的文件，该函数存在于系统函数库`unistd.h`中，其函数原型为：
>
> ```c
> int dup(int oldfd);
> ```
>
> - `oldfd`：已打开的文件描述符
> - 返回值：成功返回新的文件描述符，失败返回-1
>
> 这个新的文件描述符和原来的文件描述符共享同一个文件表项，这意味着它们共享同样的文件偏移和文件状态标志。如果在一个文件描述符上执行了`lseek()`或`write()`等操作，会影响到另一个文件描述符。

### `dup2()`函数

> `dup2()`函数用于复制一个已经打开的文件描述符到另一个文件描述符，该函数存在于系统函数库`unistd.h`中，其函数原型为：
>
> ```c
> int dup2(int oldfd, int newfd);
> ```
>
> - `oldfd`：已打开的文件描述符
> - `newfd`：新的文件描述符
> - 返回值：成功返回新的文件描述符，失败返回-1
>
> `dup2()`函数的作用是复制一个已打开的文件，复制后的文件描述符与原文件描述符指向同一个文件，它们共享同一个文件表项，但是它们有各自独立的文件指针，各自的文件读写位置互不影响
>

#### 关于两个函数的示例

```c
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#inlcude <errno.h>

int main()
{
    // 打开一个文件
    int fd = open("dup2.txt", O_RDWR | O_CREAT, 0664);
    if (fd == -1) 
    {
        fprintf();
        exit(EXIT_FAILURE);
    }
    
    // 将打开的文件描述符复制到新的文件描述符，newfd与fd指向同一个文件表项
    int newfd = dup(fd);
    if (newfd == -1) 
    {
        perror("Failed to dupliate file descriptor");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // 关闭原始的文件描述符
    close(fd);

    // 将标准输出（文件描述符为STDOUT_FILENO）重定向到新的文件描述符
    // 现在，任何写入标准输出的数据（例如通过printf函数）都将写入到这个文件中。
    if (dup2(newfd, STDOUT_FILENO) == -1) 
    {
        perror("Failed to dupliate file descriptor");
        close(newfd1);
        exit(EXIT_FAILURE);
    }

   // 现在，printf将输出到文件中
    printf("This will be written to the file\n");

    close(newfd);

    return 0;
}
```

### `fcntl()`函数

> `fcntl()`函数用于对已打开的文件进行各种控制操作，该函数存在于系统函数库`fcntl.h`中，其函数原型为：
>
> ```c
> int fcntl(int fd, int cmd, ...);
> ```
>
> - `fd`：文件描述符
> - `cmd`：控制操作的命令，常用的命令有：
>   - `F_DUPFD`：复制文件描述符
>   - `F_GETFD`：获取文件描述符标志
>   - `F_SETFD`：设置文件描述符标志
>   - `F_GETFL`：获取文件状态标志
>   - `F_SETFL`：设置文件状态标志
>   - `F_GETOWN`：获取文件拥有者
>   - `F_SETOWN`：设置文件拥有者
>   - `F_GETSIG`：获取异步I/O信号
>   - `F_SETSIG`：设置异步I/O信号
>   - `F_GETOWN_EX`：获取文件拥有者
>   - `F_SETOWN_EX`：设置文件拥有者
>   - `F_GETOWNER_UIDS`：获取文件拥有者的UID和EUID
>   - `F_NOTIFY`：设置通知
>   - `F_SETPIPE_SZ`：设置管道大小
>   - `F_GETPIPE_SZ`：获取管道大小
>   - `F_GET_RW_HINT`：获取读写提示
>   - `F_SET_RW_HINT`：设置读写提示
>   - `F_GET_FILE_RW_HINT`：获取文件读写提示
>   - `F_SET_FILE_RW_HINT`：设置文件读写提示
> - 返回值：成功返回对应的值，失败返回-1

## 操作文件

### `stat()`函数

> `stat()`函数用于获取文件的属性，该函数存在于系统函数库`sys/stat.h`中，其函数原型为：
>
> ```c
> int stat(const char *pathname, struct stat *buf);
> ```
>
> - `pathname`：文件路径名
> - `buf`：用于存储文件属性的结构体
>
> `struct stat`结构体的定义如下：
>
> ```c
> struct stat {
>     dev_t     st_dev;         /* ID of device containing file */
>     ino_t     st_ino;         /* inode number */
>     mode_t    st_mode;        /* protection */
>     nlink_t   st_nlink;       /* number of hard links */
>     uid_t     st_uid;         /* user ID of owner */
>     gid_t     st_gid;         /* group ID of owner */
>     dev_t     st_rdev;        /* device ID (if special file) */
>     off_t     st_size;        /* total size, in bytes */
>     blksize_t st_blksize;     /* blocksize for filesystem I/O */
>     blkcnt_t  st_blocks;      /* number of 512B blocks allocated */
>     time_t    st_atime;       /* time of last access */
>    time_t    st_mtime;       /* time of last modification */
>     time_t    st_ctime;       /* time of last status change */
> };
> ```
>
> - 返回值：成功返回0，失败返回-1并设置errno

#### `stat()`函数的示例

```c
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

int main()
{
    // file path
    const char *filePath = "/home/philo/professional/CSCourse/LinuxPractice/FileIO/1.txt";

    // get file status
    struct stat fileStat;
    if(stat(filePath, &fileStat) == -1)
    {
        perror("Failed to get file status");
        exit(EXIT_FAILURE);
    }

    // print file status
    printf("File size: %ld\n", fileStat.st_size);
    printf("File inode: %ld\n", fileStat.st_ino);
    printf("File mode: %o\n", fileStat.st_mode);
    printf("File nlink: %ld\n", fileStat.st_nlink);
    printf("File uid: %d\n", fileStat.st_uid);
    printf("File gid: %d\n", fileStat.st_gid);
    printf("File rdev: %ld\n", fileStat.st_rdev);
    printf("File blksize: %ld\n", fileStat.st_blksize);
    printf("File blocks: %ld\n", fileStat.st_blocks);
    printf("File atime: %ld\n", fileStat.st_atime);
    printf("File mtime: %ld\n", fileStat.st_mtime);
    printf("File ctime: %ld\n", fileStat.st_ctime);

    // print file type
    /*
    S_IFMT 是一个宏，用于分离文件类型的比特屏蔽。在 POSIX 标准中，它被定义为一个位掩码，用于提取文件类型的位。

    这两者结合起来通常用于获取文件的类型信息。通过对 st_mode 和 S_IFMT 进行按位与运算，可以提取出文件的类型部分。具体来说，st_mode 中的文件类型信息是由一系列的位所表示的，而 S_IFMT 提供了一个掩码，通过和 st_mode 进行按位与运算，可以清除文件类型信息以外的位，从而得到文件的类型。
    */
    switch (fileStat.st_mode & S_IFMT) 
    {
        case S_IFBLK:  printf("block device\n");            break;
        case S_IFCHR:  printf("character device\n");        break;
        case S_IFDIR:  printf("directory\n");               break;
        case S_IFIFO:  printf("FIFO/pipe\n");               break;
        case S_IFLNK:  printf("symlink\n");                 break;
        case S_IFREG:  printf("regular file\n");            break;
        case S_IFSOCK: printf("socket\n");                  break;
        default:       printf("unknown?\n");                break;
    }

    // print file permission
    printf("File permission: ");
    printf( (S_ISDIR(fileStat.st_mode)) ? "d" : "-");
    printf( (fileStat.st_mode & S_IRUSR) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWUSR) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXUSR) ? "x" : "-");
    printf( (fileStat.st_mode & S_IRGRP) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWGRP) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXGRP) ? "x" : "-");
    printf( (fileStat.st_mode & S_IROTH) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWOTH) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXOTH) ? "x" : "-");
    printf("\n");

    // print file last access time
    printf("File last access time: %s", ctime(&fileStat.st_atime));

    return 0;
}
```

### `rename()`函数

> `rename()`函数用于修改文件名，该函数存在于系统函数库`stdio.h`中，其函数原型为：
>
> ```c
> int rename(const char *oldpath, const char *newpath);
> ```
>
> - `oldpath`：旧文件名
> - `newpath`：新文件名
> - 该函数可以用于修改文件名，也可以用于移动文件，例如：
>   - `rename("oldpath", "newpath")`：修改文件名
>   - `rename("oldpath", "dir/newpath")`：移动文件到dir目录下并修改文件名
>   - `rename("oldpath", "dir")`：移动文件到dir目录下并保留原文件名
>   - `rename("oldpath", "dir/")`：移动文件到dir目录下并保留原文件名
>   - `rename("oldpath", "dir/newname")`：移动文件到dir目录下并修改文件名
> - 返回值：成功返回0，失败返回-1

### `remove()`函数  

> `remove()`函数用于删除一个文件，该函数存在于系统函数库`stdio.h`中，其函数原型为：
>
> ```c  
> int remove(const char *pathname);
> ```
>
> - `pathname`：文件路径名
> - 该函数可以用于删除一个文件，也可以用于删除一个空目录，但是不能删除一个非空目录
> - 该函数不能删除当前正在使用的文件，如果要删除当前正在使用的文件，可以先使用`fclose()`函数关闭文件，然后再使用`remove()`函数删除文件
> - 返回值：成功返回0，失败返回-1

### `mkdir()`函数

> `mkdir()`函数用于创建一个新的目录，该函数存在于系统函数库`sys/stat.h`中，其函数原型为：
>
> ```c
> int mkdir(const char *pathname, mode_t mode);
> ```
>
> - `pathname`：目录路径名
> - `mode`：目录的权限
> - 返回值：成功返回0，失败返回-1
>   - 该函数创建的目录权限是`mode & ~umask`，其中`umask`是进程的文件模式创建屏蔽字，它的值通常是`022`，表示屏蔽掉`group`和`other`的写权限
>   - 该函数创建的目录权限是`rwxr-xr-x`，即`755`，如果要创建其他权限的目录，可以使用`chmod()`函数修改目录的权限

### `rmdir()`函数

> `rmdir()`函数用于删除一个空目录，该函数存在于系统函数库`unistd.h`中，其函数原型为：
>
> ```c
> int rmdir(const char *pathname);
> ```
>
> - `pathname`：目录路径名
> - 返回值：成功返回0，失败返回-1
> 该函数只能删除空目录，如果要删除非空目录，可以使用`rm`命令

#### rmdir()函数的示例

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    // 删除空目录
    if (rmdir("empty_directory") == 0) {
        printf("Empty directory deleted successfully.\n");
    } else {
        perror("rmdir");
    }

    // 删除非空目录
    if (system("rm -r non_empty_directory") == 0) {
        printf("Non-empty directory deleted successfully.\n");
    } else {
        perror("rm -r");
    }

    return 0;
}
```

### `access()`函数

> `access()`函数用于检查文件的访问权限，该函数存在于系统函数库`unistd.h`中，其函数原型为：
>
> ```c
> int access(const char *pathname, int mode);
> ```
>
> - `pathname`：文件路径名
> - `mode`：访问权限，常用的访问权限有：
>   - `F_OK`：检查文件是否存在
>   - `R_OK`：检查文件是否可读
>   - `W_OK`：检查文件是否可写
>   - `X_OK`：检查文件是否可执行
> - 返回值：成功返回0，失败返回-1并设置errno

### `chmod()`函数

> `chmod()`函数用于修改文件的权限，该函数存在于系统函数库`sys/stat.h`中，其函数原型为：
>
> ```c
> int chmod(const char *pathname, mode_t mode);
> ```
>
> - `pathname`：文件路径名
> - `mode`：文件的权限
> - 返回值：成功返回0，失败返回-1  
>
> 该函数修改的文件权限是`mode & ~umask`，其中`umask`是进程的文件模式创建屏蔽字，它的值通常是`0022`，表示屏蔽掉`group`和`other`的写权限

----------------------------------------------
在 Linux 中，umask 是一种用户文件创建掩码，它用于控制新建文件的默认权限。umask 的值会从新建文件的权限掩码中减去，从而确定了新建文件的默认权限。
umask 的默认值通常为 `0022`，这意味着默认情况下新建的文件权限为 `0666 & ~0022 = 0644`，新建的目录权限为 `0777 & ~0022 = 0755`。  
可以使用 umask 命令来查看当前的 umask 值，也可以使用 umask 命令来设置新的 umask 值。例如：

```bash
umask
```

设置新的 umask 值，可以直接在命令行中使用 umask 命令，例如：

```bash
umask 0002
```
