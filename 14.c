#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <memory.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <grp.h>
#include <assert.h>
#include <signal.h>

int main() {
    const char *firstCommand[4] = {"ls", "-lR", "/", NULL};
    const char *secondCommand[3] = {"grep", "home", NULL};

    int fd[2];
    pipe(fd);

    int pid = fork();

    if(pid == -1) {
        perror("fork error");
        close(fd[0]);
        close(fd[1]);
        return 1;
    }

    if(!pid) {
        //дочерний процесс
        close(fd[0]);
        dup2(fd[1], 1);//подменим дескриптор вывода
        int err = execvp(firstCommand[0], (char **)(uint64_t)firstCommand);
        if(err == -1) {
            perror("execvp first command error");
            close(fd[1]);
            return 1;
        }
    } else {
        //родительский
        close(fd[1]);
        dup2(fd[0], 0);//подменим дескриптор ввода
        int err = execvp(secondCommand[0], (char **)(uint64_t)secondCommand);
        if(err == -1) {
            perror("execvp second command error");
            close(fd[0]);
            return 1;
        }
    }

    return 0;
}
