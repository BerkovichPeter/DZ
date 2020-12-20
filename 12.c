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
//напечатать информацию о группе
int printGroupInfo(gid_t gid) {
    struct group *group_ = getgrgid(gid);
    if(!group_) {
        perror("getgrpid error");
        return -1;
    }
    printf("gid = %d\ngroup name = %s\n", group_->gr_gid, group_->gr_name);

    return 0;
}

int printProcInfo() {
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

    //umask
    mode_t mask = umask(0);//получим старое значение маски, новое сделаем 0
    umask(mask);//присвоим обратно старое

    char *currDir = getCurrDir();
    printf("current dir = %s\n", currDir);
    free(currDir);

    printf("umask = %d\n", (int)umask);

    //id реального пользователя (группы)
    printf("---real group--\n");
    int printRes = printGroupInfo(getgid());

    if(printRes) {
        return printRes;
    }

    //id эффективного пользователя
    uid_t euid = geteuid();
    printf("euid = %d\n", euid);

    //id эффективного пользователя (группы)
    printf("---effective group---\n");

    printRes = printGroupInfo(getegid());

    if(printRes) {
        return printRes;
    }

    //получим все дополнительные идентификаторы групп процесса
    //для этого узнаем число группа, передам 0, а потом запросим это число групп
    int count = (int) getgroups(0, NULL);

    if(count == -1) {
        perror("getgroups error");
        return count;
    }

    gid_t *list = (gid_t*)calloc((size_t)count, sizeof(gid_t));

    int cnt = getgroups(count, list);

    //обработаем ошибки
    if(cnt == -1) {
        perror("getgroups error");
        free(list);
        return count;
    }

    //напечатаем информацию про все группы (дополнительные)
    //getgroups:
    printf("---all groups---\n");
    for(int i = 0; i < count; ++i) {
        printGroupInfo(list[i]);
    }
    free(list);
    return 0;
}

int main() {
    printProcInfo();
    return 0;
}
