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


void printStat(const char* pathName, const struct stat* s) {

    char Time[sizeof("00-00-0000 00:00:00 +00:00")] = "";

    printf("File %s \n",  pathName);
    printf("Size: %lld\n",  s->st_size);
    strftime(Time, sizeof(Time), "%d-%m-%Y %H:%M:%S %Z", localtime(&s->st_atime));
    printf("Access: %s\n",  Time);
    strftime(Time, sizeof(Time), "%d-%m-%Y %H:%M:%S %Z", localtime(&s->st_mtime));
    printf("Modify: %s\n",  Time);

}

int main(int argc, char **argv) {
    if(argc < 2) {
        fprintf(stderr, "need arguments\n");
    }
    //получим данные и напечатаем
    char *fileName = argv[1];

    //проверим существование файла
    if(access(fileName, F_OK)) {
        perror("file does not exist");
        return -1;
    }

    struct stat fileStat;
    int res = stat(fileName, &fileStat);
    if(res == -1) {
        perror("stat error");
        return -1;
    }

    printf("---stat---\n");
    printStat(fileName, &fileStat);

    //если файл является символьный ссылкой, то lstat будет печатать информацию о ссылке, а не о файле, на который она ссылается
    res = lstat(fileName, &fileStat);
    if(res == -1) {
        perror("lstat error");
        return -1;
    }

    printf("---lstat---\n");
    printStat(fileName, &fileStat);
    return 0;
}
