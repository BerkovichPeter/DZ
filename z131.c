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
void printGroupInfo(gid_t gid) {
    struct group *group_ = getgrgid(gid);
    if(!group_) {
        fprintf(stderr, "get grop error\n");
        return;
    }
    assert(gid == group_->gr_gid);
    printf("gid = %d\ngroup name = %s\n", group_->gr_gid, group_->gr_name);
}


void printProcInfo() {
    //id текущего процесса
    pid_t pid = getpid();
    //id родительского процесса
    pid_t ppid = getppid();
    printf("pid = %d\nppid = %d\n", pid, ppid);
    //id сеанса
    pid_t sid = getsid(0);
    //id группы процессов
    pid_t pgrp = getpgrp();
    printf("sid = %d\npgrp = %d\n", sid, pgrp);
    //id реального пользователя
    uid_t uid = getuid();
    printf("uid = %d\n", uid);
    //id реального пользователя (группы)
    printf("---real group--\n");
    printGroupInfo(getgid());

    //id эффективного пользователя
    uid_t euid = geteuid();
    printf("euid = %d\n", euid);

    //id эффективного пользователя (группы)
    printf("---effective group---\n");
    printGroupInfo(getegid());

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

int main() {
    //13.1
    //создадим новый процесс
    pid_t id = fork();
    //если создать не удалось
    if(id == -1) {
        fprintf(stderr, "fork error\n");
    }
    //выведем инфу о дочернем
    if(!id) {
        //дочерний
        printProcInfo();

    } else {
        waitpid(id, 0, 0);
    }
}