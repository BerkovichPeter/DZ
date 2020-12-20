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

ssize_t min(ssize_t a, ssize_t b) {
    return a < b ? a : b;
}

//3.1
int copyFile(const char* src, const char *dst) {
    //получим размер src  и проверим, что это файл
    struct stat srcStat;
    int statRes =stat(src, &srcStat);
    if(statRes) {
        perror("stat src error");
        return statRes;
    }
    size_t srcSz = (size_t) srcStat.st_size;

    //если src не файл
    if(!S_ISREG(srcStat.st_mode)) {
        //использую fprintf вместо perror, тк пользователю не нужно знать errno
        fprintf(stderr, "src %s isn't file", src);
    }

    struct stat dstStat;
    statRes = stat(dst, &dstStat);
    if(statRes) {
        perror("stat dst error");
        return statRes;
    }

    //откроем src файл на чтение
    int srcFD = open(src, O_RDONLY);
    if(srcFD == -1) {
        //при создании дескриптора произошла ошибка
        perror("open src error");
        return -1;
    }
    //открем dst файл на запись
    int dstFD = open(dst, O_CREAT | O_WRONLY, S_IWUSR | S_IRUSR);
    if(dstFD == -1) {
        //при создании дескриптора произошла ошибка,
        perror("open dst error");
        close(srcFD);
        return -1;
    }
    //будем копировать блоками, тк файл может не вместиться в оперативку
    size_t buffSz = 10000;
    char buff[buffSz];

    for(size_t i = 0; i < srcSz; i+=buffSz) {
        ssize_t readCount = read(srcFD, buff, buffSz);
        if(readCount == -1 ) {
            //ошибка при чтении данных из дескриптора
            perror("read error");
            close(srcFD);
            close(dstFD);
            return -1;
        }

        //обернем write в цикл
        ssize_t writed = 0;
        while(writed < readCount) {
            ssize_t w = write(dstFD, buff + writed, (size_t)(readCount - writed));

            if(w == -1) {
                perror("write error");
                return -1;
            }

            writed += w;
        }
    }
    //закроем дескрипторы
    close(srcFD);
    close(dstFD);
    return 0;
}


//3.2
//аналогична copyFile, только используем pread, pwrite, которые позволяют читать не с текущей позиции (как read, write), а со смещением
//для примера зададим src смещение 2, чтобы копировалось все кроме первых 2 символов
//смещение dst зададим на его размер, чтобы писать в конец файла
int copyFile2(const char* src, const char *dst) {
    //получим размер src  и проверим, что это файл
    struct stat srcStat;
    int statRes =stat(src, &srcStat);
    if(statRes) {
        perror("stat src error");
        return statRes;
    }
    size_t srcSz = (size_t) srcStat.st_size;

    //если src не файл
    if(!S_ISREG(srcStat.st_mode)) {
        //использую fprintf вместо perror, тк пользователю не нужно знать errno
        fprintf(stderr, "src %s isn't file", src);
    }

    struct stat dstStat;
    statRes = stat(dst, &dstStat);
    if(statRes) {
        perror("stat dst error");
        return statRes;
    }
    size_t dstSz = (size_t) dstStat.st_size;

    //откроем src файл на чтение
    int srcFD = open(src, O_RDONLY);
    if(srcFD == -1) {
        //при создании дескриптора произошла ошибка
        perror("open src error");
        return -1;
    }
    //открем dst файл на запись
    int dstFD = open(dst, O_CREAT | O_WRONLY, S_IWUSR | S_IRUSR);
    if(dstFD == -1) {
        //при создании дескриптора произошла ошибка,
        perror("open dst error");
        close(srcFD);
        return -1;
    }
    //для демонстрации pread, pwrite возьмем 10 байт src, начиная со второго символа и запишем их в конец dst
    char buff[10];

    ssize_t readCount = pread(srcFD, buff, (size_t)min((ssize_t)10, (ssize_t)srcSz - 2), 2);

    if(readCount == -1 ) {
        //ошибка при чтении данных из дескриптора
        perror("pread error");
        close(srcFD);
        close(dstFD);
        return -1;
    }

    //обернем write в цикл
    ssize_t writed = 0;
    while(writed < readCount) {
        //здесь зададим смещение, равное размеру файла, чтобы запись шла в конец
        ssize_t w = pwrite(dstFD, buff + writed, (size_t)(readCount - writed), (ssize_t)dstSz + writed);

        if(w == -1) {
            perror("pwrite error");
            return -1;
        }

        writed += w;
    }
    //закроем дескрипторы
    close(srcFD);
    close(dstFD);
    return 0;
}

int main(int argc, char **argv) {
    if(argc < 3) {
        fprintf(stderr, "need more args");
        return 1;
    }
    int copyRes1 = copyFile(argv[1], argv[2]);
    if(copyRes1) {
        return copyRes1;
    }
    int copyRes2 = copyFile2(argv[1], argv[2]);
    if(copyRes2) {
        return copyRes2;
    }
    return 0;
}
