#define _GNU_SOURCE
#include <sched.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mount.h>
#include "lib/veth.h"

#define NOT_OK_EXIT(code, msg); {if(code == -1){perror(msg); exit(-1);} }

/**
 * ���ص��޸�
 */
static void mount_root() 
{
    mount("none", "/", NULL, MS_REC|MS_PRIVATE, NULL);
    mount("proc", "/proc", "proc", 0, NULL);
}

/**
 * �������� -- ʵ��Ϊ�ӽ�������
 */
static int container_run(void *hostname)
{
    //����������
    sethostname(hostname, strlen(hostname));

    mount_root();

    /* ���½����滻�ӽ��������� */
    execlp("bash", "bash", (char *) NULL);

    //�����￪ʼ�Ĵ��뽫���ᱻִ�е�����Ϊ��ǰ�ӽ����Ѿ��������bash�滻����

    return 0;
}

static char child_stack[1024*1024]; //�����ӽ��̵�ջ�ռ�Ϊ1M

int main(int argc, char *argv[])
{
    pid_t child_pid;

    if (argc < 2) {
        printf("Usage: %s <child-hostname>\n", argv[0]);
        return -1;
    }

    /**
     * 1�������������ӽ��̣����øú����󣬸����̽���������ִ�У�Ҳ����ִ�к����waitpid
     * 2��ջ�ǴӸ�λ���λ��������������Ҫָ���λ��ַ
     * 3��SIGCHLD ��ʾ�ӽ����˳��� �ᷢ���źŸ������� �����������޹�
     * 4����������namespace
     */
    child_pid = clone(container_run,
                    child_stack + sizeof(child_stack),
                    /*CLONE_NEWUSER|*/
                    CLONE_NEWPID|CLONE_NEWNET|CLONE_NEWNS|CLONE_NEWUTS| SIGCHLD,
                    argv[1]);

    NOT_OK_EXIT(child_pid, "clone");

    /* �ȴ��ӽ��̽��� */
    waitpid(child_pid, NULL, 0);

    return 0;
}

