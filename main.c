#define _GNU_SOURCE
#include <sched.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mount.h>
#include "lib/veth.h"
#include "lib/utils.h"

#define NOT_OK_EXIT(code, msg); {if(code == -1){perror(msg); exit(-1);} }

struct container_run_para {
    char *container_ip;
    char *hostname;
    int  ifindex;
};

char* const container_args[] = {
    "/bin/sh",
    "-l",
    NULL
};

static char container_stack[1024*1024]; //�ӽ���ջ�ռ��С 1M

/**
 * ���ù��ص�
 */
static void mount_root() 
{
#if 0
    mount("none", "/", NULL, MS_REC|MS_PRIVATE, NULL);
    mount("proc", "/proc", "proc", 0, NULL);
#else    
    mount("none", "/", NULL, MS_REC|MS_PRIVATE, NULL);
    //remount "/proc" to make sure the "top" and "ps" show container's information
    if (mount("proc", "containerc_roots/rootfs/proc", "proc", 0, NULL) !=0 ) {
        perror("proc");
    }
    if (mount("sysfs", "containerc_roots/rootfs/sys", "sysfs", 0, NULL)!=0) {
        perror("sys");
    }
    if (mount("none", "containerc_roots/rootfs/tmp", "tmpfs", 0, NULL)!=0) {
        perror("tmp");
    }

    if (mount("udev", "containerc_roots/rootfs/dev", "devtmpfs", 0, NULL)!=0) {
        perror("dev");
    }
    if (mount("devpts", "containerc_roots/rootfs/dev/pts", "devpts", 0, NULL)!=0) {
        perror("dev/pts");
    }
    
    if (mount("shm", "containerc_roots/rootfs/dev/shm", "tmpfs", 0, NULL)!=0) {
        perror("dev/shm");
    }
   
    if (mount("tmpfs", "containerc_roots/rootfs/run", "tmpfs", 0, NULL)!=0) {
        perror("run");
    }
    /* 
     * ģ��Docker�Ĵ�����������mount��ص������ļ� 
     * ����Բ鿴��/var/lib/docker/containers/<container_id>/Ŀ¼��
     * ��ῴ��docker����Щ�ļ��ġ�
     */
    if (mount("./containerc_roots/conf/hosts", "./containerc_roots/rootfs/etc/hosts", "none", MS_BIND, NULL)!=0 ||
        mount("./containerc_roots/conf/hostname", "./containerc_roots/rootfs/etc/hostname", "none", MS_BIND, NULL)!=0 ||
        mount("./containerc_roots/conf/resolv.conf", "./containerc_roots/rootfs/etc/resolv.conf", "none", MS_BIND, NULL)!=0 ) {
        perror("conf 000");
    }
    /* ģ��docker run�����е� -v, --volume=[] �����ɵ��� */
    if (mount("/tmp/t1", "./containerc_roots/rootfs/mnt", "none", MS_BIND, NULL)!=0) {
        perror("mnt");
    }
    /* chroot ����Ŀ¼ */
    if (chdir("./containerc_roots/rootfs") != 0 || chroot("./") != 0){
        perror("chdir/chroot");
    }
#endif 
}

static void setnewenv() 
{
    char *penv = getenv("PATH");
    if (NULL == penv) {
        setenv("PATH", "/bin/", 1);
    } else {
        char *new_path = malloc(sizeof(char)*(strlen(penv)+32));
        sprintf(new_path, "%s:%s", penv, "/bin/");
        setenv("PATH", new_path, 1);
        free(new_path);
    }
}

/**
 * �������� -- ʵ��Ϊ�ӽ�������
 */
static int container_run(void *param)
{    
    struct container_run_para *cparam = (struct container_run_para*)param;    
    //����������
    sethostname(cparam->hostname, strlen(cparam->hostname));

    //���û�������
    setnewenv();
    mount_root();
    sleep(1);


    veth_newname("veth1", "eth0");
    veth_up("eth0");        
    veth_config_ipv4("eth0", cparam->container_ip);
    /* ���½����滻�ӽ��������� */
    execv(container_args[0], container_args);

     //�����￪ʼ�Ĵ��뽫���ᱻִ�е�����Ϊ��ǰ�ӽ����Ѿ��������sh�滻����

    return 0;
}
 
int main(int argc, char *argv[])
{
    struct container_run_para  para;
    pid_t child_pid;
    char ipv4[32] = {0};

    if (argc < 2) {
        printf("Usage: %s <child-hostname>\n", argv[0]);
        return -1;
    }
    //��ȡdocker0����ip��ַ
    
    veth_create("veth0", "veth1");
    veth_up("veth0");
    /* ��veth0���뵽docker������ */
    veth_addbr("veth0", "docker0");
    
    
    //��ȡcontainer ip
    new_containerip(ipv4, sizeof(ipv4));
    
    para.hostname = argv[1];
    para.ifindex = veth_ifindex("veth1");
    para.container_ip = ipv4;
    
    /**
     * 1�������������ӽ��̣����øú����󣬸����̽���������ִ�У�Ҳ����ִ�к����waitpid
     * 2��ջ�ǴӸ�λ���λ��������������Ҫָ���λ��ַ
     * 3��SIGCHLD ��ʾ�ӽ����˳��� �ᷢ���źŸ������� �����������޹�
     * 4����������namespace
     */
    child_pid = clone(container_run,
                      container_stack + sizeof(container_stack),
                      /*CLONE_NEWUSER|*/
                      CLONE_NEWPID|CLONE_NEWNET|CLONE_NEWNS|CLONE_NEWUTS| SIGCHLD,
                      &para);

    /* ��veth��ӵ���namespace�� */
    veth_network_namespace("veth1", child_pid);
    
    NOT_OK_EXIT(child_pid, "clone");

    /* �ȴ��ӽ��̽��� */
    waitpid(child_pid, NULL, 0);

    return 0;
}


