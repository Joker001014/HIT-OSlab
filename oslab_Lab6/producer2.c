#define __LIBRARY__   /* 在第一行添加 */
#include <stdio.h>
#include <stdlib.h>
#include <linux/sem.h>
#include <unistd.h>
#include <fcntl.h>

#define SIZE 10
#define M 510

/* add */
_syscall2(int,sem_open,const char*,name,unsigned int,value)
_syscall1(int,sem_wait,sem_t *,sem)
_syscall1(int,sem_post,sem_t *,sem)
_syscall1(int,sem_unlink,const char *,name)
_syscall2(int,shmget,int,key,int,size)
_syscall1(int, shmat, int, shmid)

int main()
{
    int shm_id;
    int count = 0;
    int *p;
    int curr;
    
    sem_t *sem_empty, *sem_full, *sem_shm;  /*3个信号量*/
    sem_empty = sem_open("empty", SIZE);
    sem_full = sem_open("full", 0);
    sem_shm = sem_open("shm", 1);

    shm_id = shmget(2521, SIZE);    /* 获取一块空闲的物理页面来创建共享内存 */
    p = (int *)shmat(shm_id);       /* 将指定物理页面映射到当前进程的虚拟内存空间 */

    /*生产多少个产品就循环几次*/
    while (count <= M) {
        /*empty大于0,才能生产*/
        sem_wait(sem_empty);    /* empty-- */
        sem_wait(sem_shm);      /* mutex-- */

        /*从上次位置继续向文件缓冲区写入一个字符*/
        curr = count % SIZE;    /*更新写入缓冲区位置，保证在0-9之间，缓冲区最大为10*/
        *(p + curr) = count;
        printf("Producer: %d\n", *(p + curr));
        fflush(stdout);

        sem_post(sem_shm);      /* mutex++ */
        sem_post(sem_full);     /* full++，唤醒消费者线程 */
        count++;
    }
    printf("producer end.\n");
    fflush(stdout);
    return 0;
}
