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

    char Time[25];
    Time[24] = 0;
    printf("File %s \n",  pathName);
    printf("Size: %ld\n",  s->st_size);
    strftime(Time, 24, "%d-%m-%Y %H:%M:%S", localtime(&s->st_atime));
    printf("Access: %s\n",  Time);
    strftime(Time, 24, "%d-%m-%Y %H:%M:%S", localtime(&s->st_mtime));
    printf("Modify: %s\n",  Time);

}

int main(int argc, char **argv) {
    if(argc < 2) {
        fprintf(stderr, "need arguments\n");
    }
    //получим данные и напечатаем
    char *fileName = argv[1];
    struct stat fileStat;
    stat(fileName, &fileStat);
    printStat(fileName, &fileStat);


    //если файл является символьный ссылкой, то lstat будет печатать информацию о ссылке, а не о файле, на который она ссылкается
    lstat(fileName, &fileStat);
    printStat(fileName, &fileStat);
    return 0;
}
