#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

void handle_error(){
    fprintf(stderr, "Failed to deal: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
}

int main()
{
    pid_t pid;
    int i;
    for (i = 0; i < 2; i++) {
        if (pid = fork() == 0) {
            break;
        }
        if (pid < 0) {
            fprintf(stderr, "Failed to fork child process: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }


    if (i == 0) {
        /*
         int execl(const char *path, const char *arg, ...);
        */
        const char *exec_path = "/bin/ls";
        const char *arg_path = "/home/philo/professional";
        int ret;
        if(ret = execl(exec_path, "ls", "-l", arg_path, (char *)NULL) == -1){
            handle_error();
        }
    }
    else if (i == 1) {
        /*int execv(const char *path, char *const argv[]);*/
        const char *exec_path = "/bin/ps";
        char *const argus[] = {"ps", "aux", (char*)NULL};
        int ret;
        if(ret = execv(exec_path, argus) == -1){
            handle_error();
        }
    }
    else {
        exit(EXIT_SUCCESS);
    }

}