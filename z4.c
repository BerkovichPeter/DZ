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
        fprintf(stderr, "need files as argument\n");
        return 1;
    }

    //откроем файловый дескриптор
    int fd = open(argv[1], O_CREAT | O_RDONLY, S_IRUSR);
    if(fd == -1) {
        fprintf(stderr, "open file error\n");
        return 1;
    }
    //отдадим rwx права группе
    if(fchmod(fd, S_IRWXG)) {
        fprintf(stderr, "change mode error\n");
        return 1;
    }

    //создадим новое время доступа и изменения файла
    struct timeval newTime [2];
    //выставим как первую секунду unix времени
    for(int i = 0; i < 2; ++i) {
        newTime[i].tv_sec = 1;
        newTime[i].tv_usec = 1;
    }

    //изменим время
    if(futimes(fd, newTime)) {
        fprintf(stderr, "change time error\n");
        return 1;
    }

    close(fd);

    return 0;
}
