#ifndef _SEM_H
#define _SEM_H

#include <linux/sched.h>
#define SEMTABLE_LEN    20   /* 信号量个数 */
#define SEM_NAME_LEN    20   /* 信号量名字长 */

typedef struct semaphore
{
    char name[SEM_NAME_LEN];   /* 信号名 */
    int value;                 /* 信号值 */
    struct task_struct *queue; /* 信号队列 */
} sem_t;

extern sem_t semtable[SEMTABLE_LEN];  /* 声明全局 */

#endif