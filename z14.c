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
#include <sys/types.h>
#include <grp.h>
#include <assert.h>
#include <signal.h>

/*
 * задача на pipe
 * напишем програму, которая принимает на вход (stdin) имена двух внешних программ (и их аргументы)
 * вызывает их и соединяет  через pipe, аналогично shell
 * например ls -lR / | grep home
 * должно работать аналогично shell
 * */


int isSepator(char c) {
    return c == ' ' || c == '\n' || c == '\t';
}

//напишем структуру, аналогичную std::vector<char*>
typedef struct vector{
    char **data_;
    size_t capacity_, size_;
} vector;

//и методы для нее
vector initVector() {
    vector res;
    res.data_ = NULL;
    res.capacity_ = res.size_ = 0;
    return res;
}
//для дебага
void printVector_(vector* v) {
    fprintf(stderr, "debug\n");
    for(size_t i = 0; i < v->size_; ++i) {
        fprintf(stderr, "%s\n", v->data_[i]);
    }
}
void resizeVector_(vector* v, size_t newCp) {
    if(newCp <= v->capacity_) {
        return;
    }
    newCp = newCp > (2 * v->capacity_) ? newCp : 2 * v->capacity_;
    char **newData = (char **)malloc(sizeof(char *) * newCp);
    memcpy(newData, v->data_, v->capacity_ * sizeof(char *));
    free(v->data_);
    v->data_ = newData;
    v->capacity_ = newCp;
}

//эта функция просто кладет указатель в вектор, поэтому удалять его извне нельзя
void pushBack(vector *v, char *s) {
    //printVector_(v);
    if(v->size_ == v->capacity_) {
        resizeVector_(v, v->capacity_ + 1);
    }
    //printVector_(v);
    v->data_[v->size_++] = s;
    //printVector_(v);
}

//удаление вектора
void deleteVector(vector* v) {
    for(size_t i = 0; i < v->size_; ++i) {
        free(v->data_[i]);
    }
    free(v->data_);
}
int main() {
    //считаем линию
    size_t argAllocSize = 0;
    char *line = NULL;
    ssize_t argSize = getline(&line, &argAllocSize, stdin);

    if(argSize == -1) {
        fprintf(stderr, "read line error");
        return 1;
    }

    vector firstArgs = initVector(), secondArgs = initVector();

    //будем парсить линию по разделителям двумя указателями

    ssize_t left = 0, right = 0;
    //сначала распарсим первую команду
    while(left < argSize) {
        for(; left < argSize && isSepator(line[left]); ++left) {

        }//ищем первый не пробел
        right = left;
        for(; right < argSize && !isSepator(line[right]); ++right) {

        }//ищем пробел
        if(right == left || left == argSize){
            break;
        }
        if(right - left == 1 && line[left] == '|') {
            left = right;
            break;
        }
        char *s = (char *) malloc(sizeof(char) * (size_t)(right - left + 1));
        memcpy(s, line + left, (size_t)(right - left));
        s[right - left] = 0;
        pushBack(&firstArgs, s);
        left = right;
    }
    //аналогично вторую
    if(left == argSize) {
        fprintf(stderr, "need 2 commands, 1 command found\n");
        free(line);
        deleteVector(&firstArgs);
        return 1;
    }
    while(left < argSize) {
        for(; left < argSize && isSepator(line[left]); ++left) {

        }//ищем первый не пробел
        right = left;
        for(; right < argSize && !isSepator(line[right]); ++right) {

        }//ищем пробел
        if(right == left || left == argSize){
            break;
        }
        char *s = (char *) malloc(sizeof(char) * (size_t)(right - left + 1));
        memcpy(s, line + left, (size_t)(right - left));
        s[right - left] = 0;
        pushBack(&secondArgs, s);
        left = right;
    }
    //обработаем случаи, если комманды не найдены
    if(secondArgs.size_ == 0) {
        fprintf(stderr, "need 2 commands, 1 command found\n");
        free(line);
        deleteVector(&firstArgs);
        deleteVector(&secondArgs);
        return 1;
    }
    if(firstArgs.size_ == 0) {
        fprintf(stderr, "need 2 commands, 0 command found\n");
        free(line);
        deleteVector(&firstArgs);
        deleteVector(&secondArgs);
        return 1;
    }

    //добавим контрольные нули
    pushBack(&firstArgs, NULL);
    pushBack(&secondArgs, NULL);
    //соединяем вывод первой команды с вводом второй
    int fd[2];
    pipe(fd);

    //создадим процесс для первой команды
    pid_t idFirst = fork();
    if(idFirst == -1) {
        fprintf(stderr, "fork error");
        free(line);
        deleteVector(&firstArgs);
        deleteVector(&secondArgs);
        return 1;
    }
    //подменяем его первой коммандой
    if(idFirst) {
        close(fd[0]);
        dup2(fd[1], 1);//подменяем stdout
        execvp(firstArgs.data_[0], firstArgs.data_);
    }
    //создаем процесс для второй
    pid_t idSecond = fork();
    if(idSecond == -1) {
        fprintf(stderr, "fork error");
        free(line);
        deleteVector(&firstArgs);
        deleteVector(&secondArgs);
        return 1;
    }
    //подменяем его
    if(idSecond) {
        close(fd[1]);
        dup2(fd[0], 0);//подменяем stdin
        execvp(secondArgs.data_[0], secondArgs.data_);
    }

    waitpid(idFirst, NULL, 0);
    waitpid(idSecond, NULL, 0);
    return 0;
}
