#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>          
#include <sys/stat.h>        
#include <semaphore.h>
#include <sys/types.h>  
#include <sys/wait.h>

#define M 530            /*打出数字总数*/
#define N 5               /*消费者进程数*/
#define BUFSIZE 10      /*缓冲区大小*/

int main()
{
    sem_t *empty, *full, *mutex;  /*3个信号量*/
    int fd;                       /*共享缓冲区文件描述符*/
    int  i,j,k,child;
    int  data;                    /*写入的数据*/
    pid_t pid;
    int  buf_out = 0;             /*记录上次从缓冲区读取位置*/
    int  buf_in = 0;              /*记录上次写入缓冲区位置*/
    /*创建信号量*/
    empty = sem_open("empty", O_CREAT|O_EXCL, 0644, BUFSIZE); /*剩余资源,初始化为size*/
    full = sem_open("full", O_CREAT|O_EXCL, 0644, 0);         /*已生产资源，初始化为0*/
    mutex = sem_open("mutex", O_CREAT|O_EXCL, 0644, 1);       /*互斥量，初始化为1*/
    /* 在文件中存储buf_out（因此生产者只有一个进程，所以buf_in不用存在文件中）*/
    fd = open("buffer.txt", O_CREAT|O_TRUNC|O_RDWR,0666); 
    lseek(fd,BUFSIZE*sizeof(int),SEEK_SET);/*修改地址偏移在10个数字之后，即40字节位置*/
    write(fd,(char *)&buf_out,sizeof(int));/*将上次读取位置buf_out存入buffer后的一个位置,每次从该位置获取上次位置，以便子进程之间通信*/
    
    /*生产者进程*/
    if((pid=fork())==0)
    {
        printf("I'm producer. pid = %d\n", getpid());
        /*生产多少个产品就循环几次*/
        for( i = 0 ; i < M; i++)
        {
            /*empty大于0,才能生产*/
            sem_wait(empty);  /* empty-- */
            sem_wait(mutex);  /* mutex-- */
            
            /*从上次位置继续向文件缓冲区写入一个字符*/
            lseek(fd, buf_in*sizeof(int), SEEK_SET); 
            write(fd,(char *)&i,sizeof(int)); 
            /*更新写入缓冲区位置，保证在0-9之间，缓冲区最大为10*/
            buf_in = (buf_in + 1) % BUFSIZE;
            
            sem_post(mutex);  /* mutex++ */
            sem_post(full);   /* full++，唤醒消费者线程 */
        }
        printf("producer end.\n");
        fflush(stdout);/*确保将输出立刻输出到标准输出。*/
        return 0;
    }
    else if(pid < 0)
    {
        perror("Fail to fork!\n");
        return -1;
    }
    
    /*消费者进程*/
    for( j = 0; j < N ; j++ )
    {
        if((pid=fork())==0)
        {
            /* 每个进程读取 M/N 个数字 */
            for( k = 0; k < M/N; k++ )
            {
                /* full大于0，才能消费 */
                sem_wait(full);  /* full-- */
                sem_wait(mutex); /* mutex-- */
                
                /*从文件第11个位置获得上次读取位置*/
                lseek(fd,BUFSIZE*sizeof(int),SEEK_SET);
                read(fd,(char *)&buf_out,sizeof(int));
                /*从上次读取位置继续读取数据*/
                lseek(fd,buf_out*sizeof(int),SEEK_SET);
                read(fd,(char *)&data,sizeof(int));
                /*在文件第11个位置写入下次应读取位置*/
                buf_out = (buf_out + 1) % BUFSIZE;
                lseek(fd,BUFSIZE*sizeof(int),SEEK_SET);
                write(fd,(char *)&buf_out,sizeof(int));

                sem_post(mutex);  /* mutex++ */
                sem_post(empty);  /* empty++，唤醒生产者进程 */
                /*消费资源*/
                printf("%d:  %d\n",getpid(),data);
                fflush(stdout);
            }
            printf("child-%d: pid = %d end.\n", j, getpid());
            return 0;
        }
        else if(pid<0)
        {
            perror("Fail to fork!\n");
            return -1;
        }
    }
    /*回收线程资源*/
    child = N + 1;
    while(child--)
        wait(NULL);
    /*释放信号量*/
    sem_unlink("full");
    sem_unlink("empty");
    sem_unlink("mutex");
    /*释放文件资源*/
    close(fd);
    return 0;
}