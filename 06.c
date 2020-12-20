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
#include <sys/param.h>
#include <time.h>

//получить директорию, в которой мы находимся сейчас
char *getCurrDir(){
    //выделим память фиксированного размера, в которую запишем директорию
    char *buff = (char *) malloc(PATH_MAX);
    if(!getcwd(buff, PATH_MAX)) {
        perror("getcwd error");
        free(buff);
        return NULL;
    }
    return buff;
}

//получить имя пользователя, который запусти программу
//необходимо, чтобы, например, обрабатывать символ ~ на входе
char *getCurrUserName(){
    //выделим память фиксированного размера, в которую запишем ответ
    char *buff = (char *) malloc(PATH_MAX);
    if(getlogin_r(buff, PATH_MAX)){
        perror("getlogin_r error");
        free(buff);
        return NULL;
    }
    return buff;
}


//мы работаем с файлами через полные имена, например /dir/dir1/file
//иногда нужно получить относительное имя файла (file)
//напишем для этого функцию
char *getFileRelativeName(const char* fullName, size_t sz) {
    if(!fullName || !sz) return NULL;

    char *lastSlash = strrchr(fullName, '/');

    if(!lastSlash || *lastSlash != '/') {
        //некоректное имя
        return NULL;
    }

    size_t id = (size_t)(lastSlash - fullName); //индекс последнего символа '/'
    char *res = (char *)malloc((sz  - id) * sizeof(char));
    memcpy(res, lastSlash + 1, sz - id - 1);
    res[sz - id - 1] = 0;
    return res;

}

//у нас может быть файл file, который находится в директори dir
//чтобы его получить нам нужен путь dir/file
//для этого хорошо бы написат функцию которая будет джойнить названия тк делать мы это будем часто
char *joinPath(const char *left, const char *right) {
    char *joined;
    int res = asprintf(&joined, "%s/%s", left, right);

    if(res == -1) {
        perror("asprintf error");
        return NULL;
    }

    return joined;
}


//уберем конструкции типа /./../ из строки *path
void refactorPathToRealPath(char **path) {
    //сначала запишем корректный путь в временный буффер res, потом скопируем оттуда в *path
    char res[PATH_MAX];
    realpath(*path, res);
    size_t sz = strlen(res);
    memcpy(*path, res, sz);
    (*path)[sz] = 0;
}

char *getFullName(const char *dir) {
    if(dir[0] == '/') {
        size_t sz = strlen(dir);
        char *res = (char *)malloc(sz * sizeof(char));
        memcpy(res, dir, sz);
        refactorPathToRealPath(&res);
        return res;
    }
    if(dir[0] == '~') {
        char *usrName = getCurrUserName();
        char *res = joinPath(usrName, dir + 2);
        free(usrName);
        refactorPathToRealPath(&res);
        return res;
    }
    char *currDir = getCurrDir();
    char *res = joinPath(currDir, dir);
    free(currDir);
    refactorPathToRealPath(&res);
    return res;
}

void printStat(const char* pathName, const struct stat* s) {

    char Time[sizeof("00-00-0000 00:00:00 +00:00")] = "";

    printf("File %s \n",  pathName);
    printf("Size: %lld\n",  s->st_size);
    strftime(Time, sizeof(Time), "%d-%m-%Y %H:%M:%S %Z", localtime(&s->st_atime));
    printf("Access: %s\n",  Time);
    strftime(Time, sizeof(Time), "%d-%m-%Y %H:%M:%S %Z", localtime(&s->st_mtime));
    printf("Modify: %s\n",  Time);

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
        perror("opendir error");
        return 1;
    }
    //6.1 обойдем директорию и выведем информацию о всех файлах
    for(struct dirent *d = readdir(dir); d; d = readdir(dir)) {
        switch (d->d_type){
            case DT_DIR:
                printf("dir: ");
                break;
            case DT_REG:
                printf("regular: ");
                break;
            case DT_BLK:
                printf("block: ");
                break;
            case DT_CHR:
                printf("character: ");
                break;
            case DT_FIFO:
                printf("fifo: ");
                break;
            case DT_LNK:
                printf("link: ");
                break;
            case DT_SOCK:
                printf("socket: ");
                break;
            case DT_UNKNOWN:
                printf("unknown: ");
                //выведем stat
                struct stat stat_;
                char *joined = joinPath(dirName, d->d_name);
                int res = stat(joined, &stat_);
                if(res) {
                    perror("stat error");
                    return res;
                }
                printStat(joined, &stat_);
                free(joined);
                break;
        }
        printf("%s\n", d->d_name);
    }
    free(dirName);
    closedir(dir);
    return 0;
}
