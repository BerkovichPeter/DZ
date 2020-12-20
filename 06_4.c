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
#include <sys/param.h>


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
    char *res = (char *)malloc((sz  - id));
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
        char *res = (char *)malloc(sz);
        memcpy(res, dir, sz);
        refactorPathToRealPath(&res);
        return res;
    }
    if(dir[0] == '~') {
        char *res = malloc(strlen(dir));
        memcpy(res, dir, strlen(dir));
        refactorPathToRealPath(&res);
        return res;
    }
    char *currDir = getCurrDir();
    char *res = joinPath(currDir, dir);
    free(currDir);
    refactorPathToRealPath(&res);
    return res;
}

void printStat(const char* pathName, const char* offsetStr, const struct stat* s) {

    char Time[sizeof("00-00-0000 00:00:00 +00:00")] = "";

    printf("%sFile %s \n", offsetStr, pathName);
    printf("%sSize: %lld\n", offsetStr, s->st_size);
    strftime(Time, sizeof(Time), "%d-%m-%Y %H:%M:%S %Z", localtime(&s->st_atime));
    printf("%sAccess: %s\n", offsetStr, Time);
    strftime(Time, sizeof(Time), "%d-%m-%Y %H:%M:%S %Z", localtime(&s->st_mtime));
    printf("%sModify: %s\n", offsetStr, Time);

}


//рекурсивная функция печати информации о всех подкаталогах директории
//аналогична 6, но если dirent является директорией, то вызывает себя рекурисвно
//вторым параметром принимает смещение, на которое будут сдвигаться данные при печати

int printDir_(const char* dirName, int offset) {
    //открываем директорию
    DIR *dir = opendir(dirName);
    if(!dir) {
        fprintf(stderr, "error open dir");
        return 1;
    }

    //строка которая задаст смещение
    char *offsetStr = (char*)malloc((size_t)offset * 4u + 1u);
    memset(offsetStr, ' ',  (size_t)offset * 4u);
    offsetStr[offset * 4] = 0;
    //6.1 обойдем директорию и выведем информацию о всех файлах
    for(struct dirent *d = readdir(dir); d; d = readdir(dir)) {
        if (strlen(d->d_name) == 1 && d->d_name[0] == '.') {
            continue;
        }
        if (strlen(d->d_name) == 2 && d->d_name[0] == '.' && d->d_name[1] == '.') {
            continue;
        }
        printf("%s", offsetStr);
        if (d->d_type == DT_DIR) {
            printf("dir: ");
            printf("%s\n", d->d_name);
            //если файл является директорией, то запустимся рекурсивно, увеличив смещение
            char *joined = joinPath(dirName, d->d_name);
            if (printDir_(joined, offset + 1)) {
                //в случае ошибки выходим
                free(joined);
                return 1;
            }
            free(joined);
            continue;
        } else if (d->d_type == DT_UNKNOWN) {
            printf("unknown: ");
            struct stat s;
            char *joined = joinPath(dirName, d->d_name);
            int resStat = stat(joined, &s);
            if (resStat) {
                perror("stat error");
                return resStat;
            }
            printStat(d->d_name, offsetStr, &s);
            free(joined);
        } else {
            switch (d->d_type) {
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
            }
        }
        printf("%s\n", d->d_name);
    }
    free(offsetStr);
    closedir(dir);
    return 0;
}

//функция печати, вызывает printDir_ со смещением 0
int printDir(const char*dirName) {
    return printDir_(dirName, 0);
}

int main(int argc, const char** argv) {
    if(argc < 2) {
        //не perror тк пользователю не нужен errno
        fprintf(stderr, "need dir as arg");
        return 1;
    }

    char *dirName = getFullName(argv[1]);

    int res = printDir(dirName);
    if(res) {
        perror("printDir error");
        return res;
    }

    return 0;
}
