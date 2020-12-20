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
//для этого хорошо бы написат функцию которая будет джойнить названия тк делать мы это будем часто
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
    char *joined = (char *)malloc(sizeof(char) * (leftSz + rightSz + 2));
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


//сжатие
void compress(char *path){
    if(!fork()){
        execlp("gzip", "gzip", "-Nr", path, NULL);
    } else{
        wait(NULL);
    }
}

//копирование файла(!) src в файл(!) dst
//в случае успешного копирования вернет 0
//по сути задание 1.0 и 1.1
//в последствии функция будет использоваться при копировании файлов при обходе директорий
int copyFile(const char* src, const char *dst) {
    //откроем src файл на чтение
    int srcFD = open(src, O_RDONLY);
    if(srcFD == -1) {
        //при создании дескриптора произошла ошибка, скорее всего, такого файла не существует
        fprintf(stderr, "error at open file %s as src, probably file doesn't exists\n", src);
        return -1;
    }
    //открем dst файл на запись
    int dstFD = open(dst, O_CREAT | O_WRONLY, S_IRWXU | S_IRWXO | S_IRWXG);
    if(dstFD == -1) {
        //при создании дескриптора произошла ошибка,
        close(srcFD);
        fprintf(stderr, "error at create file %s as dst\n", dst);
        return -1;
    }

    //будем считывать по 10000 байт из входного файла и записывать в выходной, пока не дойдем до конца
    //используем этот подход, потому что если файл очень большой, то он просто не поместится в оперативную память
    ssize_t partitionRead = 10000;
    char buff[partitionRead];
    for(;;) {
        ssize_t readCount = read(srcFD, buff, (size_t)partitionRead);
        if(readCount == -1) {
            //ошибка при чтении данных из дескриптора
            fprintf(stderr, "error read file descriptor src");
            close(srcFD);
            close(dstFD);
            return -1;
        }
        ssize_t writeCount = write(dstFD, buff, (size_t) readCount);
        if(writeCount == -1 || writeCount != readCount) {
            //ошибка при записи данных в дескриптор
            fprintf(stderr, "error write file descriptor src");
            close(srcFD);
            close(dstFD);
            return -1;
        }

        if(readCount < partitionRead) {
            //считали символов меньше чем размер порции
            //значит достигли конца файла, выходим из цикла
            break;
        }
    }
    //закроем дескрипторы
    close(srcFD);
    close(dstFD);
    return 0;
}

//скопирует файл в директорию
int copyFileToDir(const char*src, const char *dst) {
    char *fileName = getFileRelativeName(src, strlen(src));
    char *joined = joinPath(dst, fileName);
    copyFile(src, joined);
    free(fileName);
    free(joined);
    return 0;
}

//скопирует содержание src в dst
//работает рекурсивно как алгоритм дфс (обход графа в глубину)
int copyDir(const char *src, const char *dst) {
    //fprintf(stderr, "debug src = %s dst = %s\n", src, dst);
    DIR *dstDir = opendir(dst);
    if(!dstDir) {
        //ошибка при открытии директории
        //возможно файл, проверим это
        if(errno == ENOTDIR) {
            //dst - файл, копируем туда src
            return copyFile(src, dst);
        } else if(errno == ENOENT) {
            //каталога не существует
            //тогда создаем
            mkdir(dst, S_IRWXU | S_IRWXO | S_IRWXG);
            return copyDir(src, dst);
        }
        return -1;
    }
    DIR *srcDir = opendir(src);
    if(!srcDir) {
        if(errno == ENOTDIR) {
            return copyFileToDir(src, dst);
        }
        closedir(dstDir);
        return -1;
    }
    //и то и то директория
    //обходим src
    for(struct dirent * dir = readdir(srcDir); dir; dir = readdir(srcDir)) {
        //для каждого элемента в директории рекурсивно запускаем функцию
        if(dir->d_namlen == 2 &&  dir->d_name[1] == '.' && dir->d_name[0] == '.') {
            continue;
        }
        if(dir->d_namlen == 1 && dir->d_name[0] == '.') {
            continue;
        }
        char *newSrc = joinPath1(src, dir->d_name, dir->d_namlen);
        char *newDst = joinPath1(dst, dir->d_name, dir->d_namlen);
        int res = 0;
        if(dir->d_type == DT_REG) {
            //если это файл, то скопируем его в директорию
            res = copyFileToDir(newSrc, dst);
        } else if(dir->d_type == DT_DIR) {
            res = copyDir(newSrc, newDst);
        }
        free(newSrc);
        free(newDst);
        if(res) {
            //произошла ошибка
            closedir(srcDir);
            closedir(dstDir);
            return res;
        }
    }
    closedir(srcDir);
    closedir(dstDir);
    return 0;

}

//копирование, которое решит, что вызывать
int copy(const char*src, const char*dst) {
    struct stat statSrc, statDst;
    stat(src, &statSrc);
    stat(src, &statDst);
    int isDirSrc = S_ISDIR(statSrc.st_mode), isDirDst = S_ISDIR(statDst.st_mode);

    if(isDirDst && isDirSrc) {
        char *dirName = getFileRelativeName(src, strlen(src));
        char *joined = joinPath(dst, dirName);
        int res = copyDir(src, joined);
        free(dirName);
        free(joined);
        return res;
    } else if(!isDirSrc && !isDirDst) {
        return copyFile(src, dst);
    } else if(!isDirSrc) {
        return copyFileToDir(src, dst);
    } else {
        return -1;
    }

}


int main(int argc, const char **argv) {

    if(argc < 3) {//недостаточно аргументов
        fprintf(stderr, "enter the source and destination of the copy as arguments \n");
        return 1;
    }
    const char *src = argv[1];
    const char *dst = argv[2];

    if(!src) {
        fprintf(stderr, "входная директория некорректна\n");
        return 1;
    }
    if(!dst) {
        fprintf(stderr, "выходная директория некорректна\n");
        return 1;
    }
    //корректно обработаем случай текущей директори
    char *newSrc = getFullName(src);
    char *newDst = getFullName(dst);
    //скопируем
    int res = copy(newSrc, newDst);
    //int res = copyDir("/users/ikhozhaev/rt_hw/1.0", "/users/ikhozhaev/rt_hw/check");
    compress(newDst);
    //освободим память
    free(newSrc);
    free(newDst);
    if(res) {
        fprintf(stderr, "ошибка при копировании\n");
    }
    return 0;
}
