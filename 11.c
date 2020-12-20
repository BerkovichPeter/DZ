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
#include <sys/file.h>

int main() {

    char fileName[] = "counter.txt";

    int fd = open(fileName, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

    if(fd == -1) {
        perror("fd open error");
        return 1;
    }

    int code = flock(fd, LOCK_EX);//создадим экслюзивную блокировку файла
    if(code == -1) {
        close(fd);
        perror("flock error");
    }

    //читаем и пишем
    char buff[18];
    memset(buff, 0, 18);
    read(fd, buff, 18);
    int startCount = atoi(buff);
    ++startCount;
    lseek(fd, 0, SEEK_SET);//сместим позицию в начало, чтобы записывать в начало
    dprintf(fd, "%d ", startCount);

    flock(fd, LOCK_UN);//разблокируем файл
    close(fd);
    return 0;
}
