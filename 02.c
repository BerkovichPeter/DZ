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
#include <assert.h>

int main(int argc, char **argv) {
    assert(argc == 3);

    //откроем файловый дескриптор на запись (если файла нет, то создаем)
    int fd = open(argv[1], O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);

    if(fd == -1) {
        perror("error open file");
    }

    //2.1
    //будем записывать пока не запишется все сообщение (если write будет записывать по 1 байту)
    size_t writed = 0, sz = strlen(argv[2]);
    while(writed < sz) {
        ssize_t w = write(fd, argv[2] + writed, sz - writed);
        if(w < 0) {
            perror("write error");
            close(fd);
            return 1;
        }
        writed += (size_t) w;
    }

    //2.2
    //используем dprintf (аналог printf, но пишет в дескриптор)
    int e = dprintf(fd, "%d %f %s", 7, 2.1, argv[2]);
    if(e < 0) {
        perror("dprintf error");
        close(fd);
        return -1;
    }
    //закрываем дескриптор
    close(fd);

    return 0;
}
