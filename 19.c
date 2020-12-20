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
#include <pthread.h>


const size_t threadCount = 5;
pthread_mutex_t mutex;
size_t iterationNeed = 50;
size_t iterationCount = 1;

double res = 0;
double x = 0.5;
double myX;

//вычисляем ln(1 + x)
void *threading(void *num) {
    if(num)
        return NULL;
    while(iterationCount < iterationNeed) {
        pthread_mutex_lock(&mutex);
        if(iterationCount == iterationNeed) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        res += myX / (double)iterationCount;

        myX *= -x;
        ++iterationCount;
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main() {
    pthread_t threads[threadCount];
    pthread_mutex_init(&mutex, NULL);
    myX = x;

    for(size_t i = 0; i < threadCount; ++i) {
        int createRes = pthread_create(&threads[i], NULL, threading, NULL);
        if(createRes) {
            //в случае ошибки создания заджойним все потоки и  выйдем аварийно
            perror("pthread create error");
            for(size_t j = 0; j < i; ++j) {
                int res = pthread_join(threads[j], NULL);
                if(res) {
                    perror("join error");
                    return res;
                }
            }
            return createRes;
        }
    }
    //заджойним все потоки и выйдем
    for(size_t j = 0; j < threadCount; ++j) {
        int res = pthread_join(threads[j], NULL);
        if(res) {
            perror("join error");
            return res;
        }
    }
    printf("%f", res);
    return 0;
}
