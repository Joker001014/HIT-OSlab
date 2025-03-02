#include <asm/segment.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <errno.h>

#define _SHM_NUM 20

/* 共享内存结构体 */
struct shm_tables
{
	int key;				/* 共享内存标识 */
	int size;				/* 共享内存大小 */
	unsigned long page;		/* 共享内存地址 */
} shm_tables[_SHM_NUM];


/* 获取一块空闲的物理页面来创建共享内存 */
int sys_shmget(int key, int size)
{
    int i;
    unsigned long page;		/* 存放共享内存地址 */

	/* 查看 key 对应的共享内存是否已存在 */
    for (i = 0; i < _SHM_NUM; i++) 
        if(shm_tables[i].key == key)
            return i;
	
	/* 内存大小超过一页 */
    if (size > PAGE_SIZE) 
        return -EINVAL;

	/* 获取一块空闲物理内存页面，返回起始物理地址 */
    page = get_free_page(); 
    if(!page)
        return -ENOMEM;

	/* 记录到共享内存表中 */
    for (i = 0; i < _SHM_NUM; i++) {
        if(shm_tables[i].key == 0) {
            shm_tables[i].key = key;
            shm_tables[i].size = size;
            shm_tables[i].page = page;
            return i;
        }
    }
    return -1;  /* 共享内存数量已满 */
}

/* 将指定物理页面映射到当前进程的虚拟内存空间 */
void * sys_shmat(int shmid)
{
    int i;
    unsigned long data_base;

	/* 判断共享内存 shmid 是否越界 及 共享内存是否存在 */
    if (shmid < 0 || shmid >= _SHM_NUM || shm_tables[shmid].key == 0)
        return -EINVAL;
	
	/* 把物理页面映射到进程的虚拟内存空间，映射到代码段+数据段后，堆栈段前 */
    put_page(shm_tables[shmid].page, current->brk + current->start_code);
	/* 修改总长度，brk为代码段和数据段的总长度 */
    current->brk += PAGE_SIZE;
    return (void*)(current->brk - PAGE_SIZE);
}












