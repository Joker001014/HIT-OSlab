// sys_iam 和 sys_whoami 系统调用函数

#include <string.h>
#include <errno.h>
#include <asm/segment.h>

// 内核中存放name，23字符 + '\0' = 24
char msg[24];

// 将字符串参数 name 拷贝到内核中保存下来
// return：拷贝的字符数。如果name的字符个数超过了23,则返回“­-1”,并置errno为EINVAL。
int sys_iam(const char* name) 
{
    int i;
    char tmp[30];   // 临时存储，操作失败不影响 msg
    
    // 从用户态内存获取数据，存入 tmp
    for(i = 0; i < 30; i++) {
        tmp[i] = get_fs_byte(name+i);
        if(tmp[i] == '\0')
            break;
    }
    // printk(tmp);

    i = 0;
    // 计算 name 的长度
    while(tmp[i] != '\0' && i < 30) {
        i++;
    }
    int len = i;
    // 长度大于 23 返回报错
    if(len > 23) {
        return -(EINVAL);
    }
    strcpy(msg, tmp);

    return i;
}

// 将由 iam() 保存的名字拷贝到 name 指向的用户地址空间，size 指定 name 的大小
// return：如果size小于需要的空间,则返回“­-1”,并置errno为 EINVAL
int sys_whoami(char* name, unsigned int size)
{ 	
    int len = 0;
    // 计算长度
    for(; msg[len] != '\0'; len++);
    // 长度大于则返回
    if(len > size) {
        return -(EINVAL);
    }
    // 从内核态获取数据，存入 name
    int i = 0;
    for(; i < size; i++) {
        put_fs_byte(msg[i], name+i);
        if(msg[i] == '\0') 
            break;
    }
    return i;
}
