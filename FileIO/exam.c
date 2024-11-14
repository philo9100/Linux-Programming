#include "asm/string_64.h"
#include "linux/types.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFFER_SIZE 1024    // 缓冲区大小

int main()
{
    const char *path_name = "/home/philo/professional/CSCourse/LinuxPractice/FileIO/1.txt";

    const char *content = "hello world!";

    int fd; // 文件描述符
    open(path_name, O_RDWD | O_CLOEXEC | O_CREAT | O_EXCL | O_TRUNC, 0777);

    if(fd == -1)
    {
        fprintf(stderr, "Failed to open file: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Open file successfully\n");
    }

    ssize_t bytes_write;
    bytes_write = write(fd, content, strlen(content)); 

    if (bytes_write == -1) {
        fprintf(stderr, "failed to write file: %s\n", strerror(errno));
        close(fd);
        exit(EXIT_FAILURE);
    }
    else 
    {
        printf("Write content to file successfully\n");
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    bytes_read = read(fd, buffer, BUFFER_SIZE);

    if(bytes_read == -1)
    {
        fprintf(stderr, "failed to read file: %s\n", strerror(errno));
        close(fd);
        exit(EXIT_FAILURE);
    }
    
    if(lseek(fd, 0, SEEK_SET) == -1) // 
    {
        fprintf(stderr, "failed to lseek file: %s\n", strerror(errno));
        close(fd);
        exit(EXIT_FAILURE);
    }

    const char *new_path_name;
    int new_fd = open(new_path_name, O_RDWD | O_CLOEXEC | O_CREAT | O_TRUNC, 0777);
    if (new_fd == -1) {
        fprintf(stderr, "Failed to open new file: %s\n", strerror(errno));
        close(fd);
        exit(EXIT_FAILURE);
    }


    while (bytes_read = read(fd, buffer, BUFFER_SIZE) > 0) {
        if (write(new_fd, buffer, bytes_read) == -1){
            fprintf(stderr, "failed to write: %s\n", strerror(errno));
            close(fd);
            close(new_fd);
            exit(EXIT_FAILURE);
        }
    }



}