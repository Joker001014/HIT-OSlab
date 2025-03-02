#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>

#define SIZE 10
#define M 510

int main()
{
    int shm_id;
    int count = 0;
    int *p;
    int curr;
    sem_t *sem_empty, *sem_full, *sem_shm;
    sem_empty = sem_open("empty", O_CREAT|O_EXCL, 0644, SIZE);
    sem_full = sem_open("full", O_CREAT|O_EXCL, 0644, 0);
    sem_shm = sem_open("shm",  O_CREAT|O_EXCL, 0644, 1);
    shm_id = shmget(2521, SIZE, IPC_CREAT | IPC_EXCL | 0664); // 创建共享内存
    p = (int *)shmat(shm_id, NULL, 0);
    while (count <= M) {
        sem_wait(sem_empty);
        sem_wait(sem_shm);
        curr = count % SIZE;
        *(p + curr) = count;
        printf("Producer: %d\n", *(p + curr));
        fflush(stdout);
        sem_post(sem_shm);
        sem_post(sem_full);
        count++;
    }
    printf("producer end.\n");
    fflush(stdout);
    return 0;
}
