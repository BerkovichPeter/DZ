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

//напечатать информацию о группе
//теперь будет выводить в файл
void printGroupInfo(gid_t gid, FILE *f) {
    struct group *group_ = getgrgid(gid);
    if(!group_) {
        fprintf(stderr, "get grop error\n");
        return;
    }
    assert(gid == group_->gr_gid);
    fprintf(f, "gid = %d\ngroup name = %s\n", group_->gr_gid, group_->gr_name);
}


void printProcInfo(FILE *f) {
    //fprintf(f, "debug1 = %f\n", clock() * 1.0 / CLOCKS_PER_SEC);
    //id текущего процесса
    pid_t pid = getpid();
    //id родительского процесса
    pid_t ppid = getppid();
    fprintf(f, "pid = %d\nppid = %d\n", pid, ppid);
    //id сеанса
    pid_t sid = getsid(0);
    //id группы процессов
    pid_t pgrp = getpgrp();
    fprintf(f, "sid = %d\npgrp = %d\n", sid, pgrp);
    //id реального пользователя
    uid_t uid = getuid();
    fprintf(f, "uid = %d\n", uid);
    //id реального пользователя (группы)
    fprintf(f, "---real group--\n");
    printGroupInfo(getgid(), f);

    //id эффективного пользователя
    uid_t euid = geteuid();
    fprintf(f, "euid = %d\n", euid);

    //id эффективного пользователя (группы)
    fprintf(f, "---effective group---\n");
    printGroupInfo(getegid(), f);

    //получим все дополнительные идентификаторы групп процесса
    //для этого запросим максимальное число возможных групп процесса
    /*gid_t *list = (gid_t*)malloc((unsigned) sysconf(_SC_NGROUPS_MAX) * sizeof(gid_t));
    int count = (int) getgroups((int)sysconf(_SC_NGROUPS_MAX), list);

    //обработаем ошибки
    if(count == -1) {
        fprintf(stderr, "get groups error:");
        if(errno == EFAULT){
            fprintf(stderr, "EFAULT");
        } else if(errno == EINVAL) {
            fprintf(stderr, "EINVAL");
        } else if(errno == EPERM) {
            fprintf(stderr, "EPERM");
        } else {
            fprintf(stderr, "unknown error");
        }
        fprintf(stderr, "\n");
        return;
    }

    //напечатаем информацию про все группы (дополнительные)
    //getgroups:
    printf("---all groups---\n");
    for(int i = 0; i < count; ++i) {
        printGroupInfo(list[i]);
    }*/

}

FILE *f;

//функция обработки сигнала
void funcAction() {
    printProcInfo(f);
    fclose(f);//закроем файл
    exit(0);
}

int main() {
    //13.2
    //создадим новый процесс
    pid_t id = fork();
    //если создать не удалось
    if(id == -1) {
        fprintf(stderr, "fork error\n");
    }
    //выведем инфу о дочернем
    if(!id) {
        //опишем, что делать после получения сигнала
        (void)signal(SIGINT, funcAction);
        //тк второй раз функция вывода будет вызвана после завершения родительского процесса
        //мы не увидим stdout
        //поэтому сделаем вывод в файл
        f = fopen("output", "w+");
        printProcInfo(f);
        for(;;);

    } else {
        sleep(1);
        //пошлем сигнал ^C дочернему процессу
        kill(id, SIGINT);
        exit(0);
    }
}