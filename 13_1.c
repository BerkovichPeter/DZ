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
        perror("getGroupError");
        return;
    }
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

}

int main() {
    //13.1
    //создадим новый процесс
    pid_t id = fork();
    //если создать не удалось
    if(id == -1) {
        perror("fork error");
    }
    //выведем инфу о дочернем
    if(!id) {
        //дочерний
        printf("---children---\n");
        printProcInfo();

    } else {
        int res = waitpid(id, 0, 0);
        if(res == -1) {
            perror("waitpid error");
            return 1;
        }
        printf("\n---parent---\n");
        printProcInfo();
        return 0;
    }
}