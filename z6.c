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


//получить директорию, в которой мы находимся сейчас
char *getCurrDir(){
    //выделим память фиксированного размера, в которую запишем директорию
    size_t buffSz = 20;
    char *buff = (char *) malloc(sizeof(char) * buffSz);

    //если имя директории длинное, то памяти может не хватить
    //в таком случае будем увеличивать объем памяти в 2 раза и пытаться получить директорию вызовом getcwd
    //будем повторять пока не хватит памяти
    while(!getcwd(buff, buffSz)){
        buffSz *= 2;
        free(buff);
        buff = (char *) malloc(sizeof(char) * buffSz);
    }
    //вернем указатель на имя директории
    return buff;
}

//получить имя пользователя, который запусти программу
//необходимо, чтобы, например, обрабатывать символ ~ на входе
char *getCurrUserName(){
    //выделим память фиксированного размера, в которую запишем ответ
    size_t buffSz = 20;
    char *buff = (char *) malloc(sizeof(char) * buffSz);

    //аналогично функции getCurrDir
    while(!getlogin_r(buff, buffSz)){
        buffSz *= 2;
        free(buff);
        buff = (char *) malloc(sizeof(char) * buffSz);
    }
    //вернем указатель на имя пользователя
    return buff;
}


//мы работаем с файлами через полные имена, например /dir/dir1/file
//иногда нужно получить относительное имя файла (file)
//напишем для этого функцию
char *getFileRelativeName(const char* fullName, size_t sz) {
    if(!fullName || !sz) return NULL;
    size_t left = sz - 1;

    while(left && fullName[left] != '/') --left;

    if(fullName[left] != '/') {
        //некоректное имя
        return NULL;
    }

    char *res = (char *)malloc((sz  - left) * sizeof(char));
    memcpy(res, fullName + left + 1, sz - left - 1);
    res[sz - left - 1] = 0;
    return res;

}

//у нас может быть файл file, который находится в директори dir
//чтобы его получить нам нужен путь dir/file
//написать функцию которая будет джойнить названия 
char *joinPath(const char *left, const char *right) {
    //получим размеры названий
    size_t leftSz = strlen(left), rightSz = strlen(right);
    //выделим память для результата
    char *joined = (char *)malloc(sizeof(char) * (leftSz + rightSz + 2));
    //скопируем названия
    memcpy(joined, left, leftSz);
    joined[leftSz] = '/';
    memcpy(joined + leftSz + 1, right, rightSz);
    joined[leftSz + rightSz + 1] = 0;

    return joined;
}
//аналогично joinPath, но размер правой директории задан заранее, его не надо вычислять
//полезно для
char *joinPath1(const char *left, const char *right, size_t rightSz) {
    //получим размеры названий
    size_t leftSz = strlen(left);
    //выделим память для результата
    char *joined = (char *)malloc(sizeof(char) * (leftSz + rightSz + 1));
    //скопируем названия
    memcpy(joined, left, leftSz);
    joined[leftSz] = '/';
    memcpy(joined + leftSz + 1, right, rightSz);
    joined[leftSz + rightSz + 1] = 0;

    return joined;
}

char *getFullName(const char *dir) {
    if(dir[0] == '/') {
        size_t sz = strlen(dir);
        char *res = (char *)malloc(sz * sizeof(char));
        memcpy(res, dir, sz);
        return res;
    }
    if(dir[0] == '~') {
        char *usrName = getCurrUserName();
        char *res = joinPath(usrName, dir + 2);
        free(usrName);
        return res;
    }
    char *currDir = getCurrDir();
    char *res = joinPath(currDir, dir);
    free(currDir);
    return res;
}



int main(int argc, const char **argv) {
    if(argc < 2) {
        fprintf(stderr, "need dir as arg");
        return 1;
    }

    char *dirName = getFullName(argv[1]);

    //открываем директорию
    DIR *dir = opendir(dirName);
    if(!dir) {
        fprintf(stderr, "error open dir");
        return 1;
    }
    //6.1 обойдем директорию и выведем информацию о всех файлах
    for(struct dirent *d = readdir(dir); d; d = readdir(dir)) {
        if(d->d_type == DT_DIR) {
            printf("dir: ");
        } else if(d->d_type == DT_REG) {
            printf("regular: ");
        } else if(d->d_type == DT_BLK) {
            printf("block: ");
        } else if(d->d_type == DT_CHR) {
            printf("character: ");
        } else if(d->d_type == DT_FIFO) {
            printf("fifo: ");
        } else if(d->d_type == DT_LNK) {
            printf("link ");
        } else if(d->d_type == DT_SOCK) {
            printf("socket ");
        } else if(d->d_type == DT_UNKNOWN) {
            printf("unknown: ");
        }
        printf("%s\n", d->d_name);
    }
    free(dirName);
    closedir(dir);
    return 0;
}
