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

int main() {
    //2.1
    //откроем файловый дескриптор на запись (если файла нет, то создаем)
    const char file [] = "myFile";
    int fd = open(file, O_CREAT | O_WRONLY, S_IRUSR);
    const char message [] = "message";
    //запишем туда сообщение
    write(fd, message, strlen(message));
    //закроем дескриптор
    close(fd);


    //2.2
    //используем dprintf (аналог printf, но пишет в дескриптор)
    const char file2 [] = "mySecondFile";
    //открываем
    fd = open(file2, O_CREAT | O_WRONLY, S_IRUSR);
    //пишем
    dprintf(fd, "%d %f %s", 7, 2.1, message);
    //закрываем
    close(fd);


    return 0;
}
