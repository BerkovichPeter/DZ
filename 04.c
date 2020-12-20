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
#include <sys/time.h>

int main(int argc, char **argv) {
    //первым аргументом передаем файл

    if(argc < 2) {
        //используем здесь не perror, потому что пользователю не надо знать ничего о значении errno
        fprintf(stderr, "need file as argument\n");
        return 1;
    }

    //откроем файловый дескриптор
    int fd = open(argv[1], O_CREAT | O_RDONLY, S_IRUSR);
    if(fd == -1) {
        perror("open file error");
        return -1;
    }
    //отдадим rwx права группе
    int fchmodRes = fchmod(fd, S_IRWXG);
    if(fchmodRes) {
        perror("change mode error");
        close(fd);
        return fchmodRes;
    }

    //создадим новое время доступа и изменения файла
    struct timespec newTime [2];
    //выставим как вторую секунду unix времени
    for(int i = 0; i < 2; ++i) {
        newTime[i].tv_sec = 2;
        newTime[i].tv_nsec = 10;
    }

    //изменим время
    int futimensRes = futimens(fd, newTime);

    if(futimensRes) {
        perror("change time error");
        close(fd);
        return futimensRes;
    }

    close(fd);
    return 0;
}
