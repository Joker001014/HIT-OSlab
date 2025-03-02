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
    struct shmid_ds buf;
    int *p;
    int curr;
    sem_t *sem_empty, *sem_full, *sem_shm;
    sem_empty = sem_open("empty", SIZE);
    sem_full = sem_open("full", 0);
    sem_shm = sem_open("shm", 1);
    shm_id = shmget(2521, 0, 0);
    p = (int *)shmat(shm_id, NULL, 0);
    while(count <= M) {
        sem_wait(sem_full);
        sem_wait(sem_shm);
        curr = count % SIZE;
        printf("%d:%d\n", getpid(), *(p + curr));
        fflush(stdout);
        sem_post(sem_shm);
        sem_post(sem_empty);
        count++;
    }
    printf("consumer end.\n");
    fflush(stdout);
    sem_unlink("empty");
    sem_unlink("full");
    sem_unlink("shm");
    shmctl(shm_id, IPC_RMID, &buf);
    return 0;
}
