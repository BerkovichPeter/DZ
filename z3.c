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

//3.1
int copyFile(const char* src, const char *dst) {
    //получим размер src  и проверим, что это файл
    struct stat srcStat;
    stat(src, &srcStat);
    size_t srcSz = (size_t) srcStat.st_size;

    //если src не файл
    if(!S_ISREG(srcStat.st_mode)) {
        fprintf(stderr, "src %s isn't file", src);
    }


    //откроем src файл на чтение
    int srcFD = open(src, O_RDONLY, S_IRUSR);
    if(srcFD == -1) {
        //при создании дескриптора произошла ошибка
        fprintf(stderr, "error at open file %s as src, probably file doesn't exists\n", src);
        return -1;
    }
    //открем dst файл на запись
    //trunc флаг нужен для того, чтобы урезать длину файла до 0, если он существует
    int dstFD = open(dst, O_CREAT | O_WRONLY | O_TRUNC, S_IWUSR);
    if(dstFD == -1) {
        //при создании дескриптора произошла ошибка,
        close(srcFD);
        fprintf(stderr, "error at create file %s as dst\n", dst);
        return -1;
    }
    //аналогичная проверка для dst
    //эту проверку мы делаем после open, потому что dst могло не существовать
    struct stat dstStat;
    stat(dst, &dstStat);

    //если dst не файл
    if(!S_ISREG(dstStat.st_mode)) {
        fprintf(stderr, "dst %s isn't file", dst);
    }
    //скопируем srcSz байт
    char *buff = (char *) malloc(srcSz);
    ssize_t readCount = read(srcFD, buff, srcSz);
    if(readCount == -1 || (size_t) readCount != srcSz) {
        //ошибка при чтении данных из дескриптора
        fprintf(stderr, "error read file descriptor src");
        close(srcFD);
        close(dstFD);
        return -1;
    }
    ssize_t writeCount = write(dstFD, buff, (size_t)readCount);
    if(writeCount == -1 || writeCount != readCount) {
        //ошибка при записи данных в дескриптор
        fprintf(stderr, "error write file descriptor src");
        close(srcFD);
        close(dstFD);
        return -1;
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
    stat(src, &srcStat);
    size_t srcSz = (size_t) srcStat.st_size;

    //если src не файл
    if(!S_ISREG(srcStat.st_mode)) {
        fprintf(stderr, "src %s isn't file", src);
    }


    //откроем src файл на чтение
    int srcFD = open(src, O_RDONLY, S_IRUSR);
    if(srcFD == -1) {
        //при создании дескриптора произошла ошибка
        fprintf(stderr, "error at open file %s as src, probably file doesn't exists\n", src);
        return -1;
    }
    //открем dst файл на запись
    int dstFD = open(dst, O_CREAT | O_WRONLY, S_IWUSR);
    if(dstFD == -1) {
        //при создании дескриптора произошла ошибка,
        close(srcFD);
        fprintf(stderr, "error at create file %s as dst\n", dst);
        return -1;
    }
    //аналогичная проверка для dst
    //эту проверку мы делаем после open, потому что dst могло не существовать
    struct stat dstStat;
    stat(dst, &dstStat);

    //если dst не файл
    if(!S_ISREG(dstStat.st_mode)) {
        fprintf(stderr, "dst %s isn't file", dst);
    }
    //скопируем srcSz байт
    char *buff = (char *) malloc(srcSz);
    ssize_t readCount = pread(srcFD, buff, srcSz - 2, 2);
    if(readCount == -1 || readCount != ((ssize_t)srcSz - 2)) {
        //ошибка при чтении данных из дескриптора
        fprintf(stderr, "error read file descriptor src");
        close(srcFD);
        close(dstFD);
        return -1;
    }
    //здесь зададим смещение, равное размеру файла, чтобы запись шла в конец
    ssize_t writeCount = pwrite(dstFD, buff, (size_t)readCount, dstStat.st_size);
    if(writeCount == -1 || writeCount != readCount) {
        //ошибка при записи данных в дескриптор
        fprintf(stderr, "error write file descriptor src");
        close(srcFD);
        close(dstFD);
        return -1;
    }
    //закроем дескрипторы
    close(srcFD);
    close(dstFD);
    return 0;
}

int main() {
    if(copyFile("in.txt", "out.txt")) {
        fprintf(stderr, "copy error\n");
    }
    if(copyFile2("in.txt", "out.txt")) {
        fprintf(stderr, "copy error\n");
    }
    return 0;
}
