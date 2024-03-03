
#include "interrupt.h"
#include "../include/fs.h"

extern void initSem(uint16_t __value, int32_t *semId);
extern bool sem_open(uint32_t semId);
extern void semWait(int32_t semId);
extern void semSignal(int32_t semId);

extern void yeid();
extern void exit_pro();

typedef struct
{
    char *buf;
    uint32_t typeId;
    uint32_t deviceId;
    uint32_t addr;
    uint32_t size;
} devParam;

typedef struct
{
    uint32_t fd;
    uint32_t addr;
    char *buf;
    uint32_t size;
} file_param;

extern uint32_t
open(uint32_t typeId, uint32_t deviceId);
extern int32_t read(uint32_t typeId, uint32_t deviceId, uint32_t addr, char *buf, uint32_t size);
extern int32_t write(uint32_t typeId, uint32_t deviceId, uint32_t addr, char *buf, uint32_t size);
extern uint32_t control(uint32_t typeId, uint32_t deviceId, uint32_t cmd, int32_t *args, uint32_t n);
extern void close(uint32_t typeId, uint32_t deviceId);
extern void info(uint32_t typeId, uint32_t deviceId, char *buff);
extern int32_t info_by_index(int32_t index, char *buff);

void sys_call(StackInfo *s)
{

    // file_param *fp;
    // switch (s->EAX)
    // {
    // case 1: // 输出中断
    //     log_qemu_printf(s->EBX);
    //     break;
    //     // case 10: // 创建进程
    //     //     log_qemu_printf(s->EBX);
    //     // break;
    // case 11: // yeid,主动让出cpu
    //     yeid();
    //     break;
    // case 12:
    //     exit_pro();
    //     break;
    // case 50: // 信号量初始化
    //     // ebx==>value
    //     // ecx==>*semId
    //     initSem(s->EBX, s->ECX);
    //     break;
    // case 51:
    //     // ebx==>semId
    //     sem_open(s->EBX);
    //     break;
    // case 52:
    //     // ebx==>semId
    //     semWait(s->EBX);
    //     break;
    // case 53:
    //     // ebx==>semId
    //     semSignal(s->EBX);
    //     break;
    // case 60: // 设备管理相关系统调用
    //          // ebx==>devParam
    //          // ecx==>返回参数地址
    //     *(uint32_t *)(s->ECX) = open(((devParam *)(s->EBX))->typeId, ((devParam *)(s->EBX))->deviceId);
    //     break;
    // case 61:
    //     // ebx==>devParam
    //     // ecx==>返回参数地址
    //     *(uint32_t *)(s->ECX) = read(((devParam *)(s->EBX))->typeId, ((devParam *)(s->EBX))->deviceId, ((devParam *)(s->EBX))->addr, ((devParam *)(s->EBX))->buf, ((devParam *)(s->EBX))->size);
    //     break;
    // case 62:
    //     // ebx==>devParam
    //     // ecx==>返回参数地址
    //     *(uint32_t *)(s->ECX) = write(((devParam *)(s->EBX))->typeId, ((devParam *)(s->EBX))->deviceId, ((devParam *)(s->EBX))->addr, ((devParam *)(s->EBX))->buf, ((devParam *)(s->EBX))->size);
    //     break;
    // case 63:
    //     // *(uint32_t *)(s->ECX) = write(devPar->typeId, devPar->deviceId, devPar->addr, devPar->buf, devPar->size);
    //     break;
    // case 64:
    //     // ebx==>devParam
    //     // ecx==>返回参数地址
    //     close(((devParam *)(s->EBX))->typeId, ((devParam *)(s->EBX))->deviceId);
    //     break;
    // case 65:
    //     // ebx==>devParam
    //     // ecx==>返回参数地址
    //     info(((devParam *)(s->EBX))->typeId, ((devParam *)(s->EBX))->deviceId, ((devParam *)(s->EBX))->buf);
    //     break;
    // case 66:
    //     // ebx=int32_t* index
    //     // ecx=char *buff
    //     *(uint32_t *)(s->ECX) = info_by_index(*(uint32_t *)(s->ECX), s->EBX);
    //     break;
    // // 文件系统相关,系统调用
    // case 100:
    //     // 打开 ebx存放名称
    //     *(bool *)(s->ECX) = syscall_fs_open(s->EBX);
    //     break;
    // case 101:
    //     // 读
    //     fp = (file_param *)(s->EBX);
    //     if (fp != NULL)
    //     {
    //         *(uint32_t *)(s->ECX) = syscall_read_fs(fp->fd, fp->addr, fp->buf, fp->size);
    //     }
    //     break;
    // case 102:
    //     // 写
    //     fp = (file_param *)(s->EBX);
    //     if (fp != NULL)
    //     {
    //         *(uint32_t *)(s->ECX) = syscall_write_fs(fp->fd, fp->addr, fp->buf, fp->size);
    //     }
    //     break;
    // case 103:
    //     // 关闭
    //     syscall_close_fs(s->EBX);
    //     break;
    // case 104:
    //     // 文件夹相关操作 在某个文件夹下创建文件夹
    //     // fp.buff中存放要创建文件夹名称
    //     fp = (file_param *)(s->EBX);
    //     if (fp != NULL)
    //     {
    //         *(uint32_t *)(s->ECX) = make_dir(fp->fd, fp->buf);
    //     }
    //     break;
    // case 105:
    //     // 创建文件
    //     fp = (file_param *)(s->EBX);
    //     if (fp != NULL)
    //     {
    //         *(uint32_t *)(s->ECX) = make_file(fp->fd, fp->buf);
    //     }
    //     break;
    // default:
    //     break;
    // }
    file_param *fp;
    if (s->EAX == 1)
    { // 输出中断
        log_qemu_printf(s->EBX);
    }
    else if (s->EAX == 11)
    { // yeid,主动让出cpu
        yeid();
    }
    else if (s->EAX == 12)
    {
        exit_pro();
    }
    else if (s->EAX == 50)
    { // 信号量初始化
        initSem(s->EBX, s->ECX);
    }
    else if (s->EAX == 51)
    {
        sem_open(s->EBX);
    }
    else if (s->EAX == 52)
    {
        semWait(s->EBX);
    }
    else if (s->EAX == 53)
    {
        semSignal(s->EBX);
    }
    else if (s->EAX == 60)
    { // 设备管理相关系统调用
        *(uint32_t *)(s->ECX) = open(((devParam *)(s->EBX))->typeId, ((devParam *)(s->EBX))->deviceId);
    }
    else if (s->EAX == 61)
    {
        *(uint32_t *)(s->ECX) = read(((devParam *)(s->EBX))->typeId, ((devParam *)(s->EBX))->deviceId, ((devParam *)(s->EBX))->addr, ((devParam *)(s->EBX))->buf, ((devParam *)(s->EBX))->size);
    }
    else if (s->EAX == 62)
    {
        *(uint32_t *)(s->ECX) = write(((devParam *)(s->EBX))->typeId, ((devParam *)(s->EBX))->deviceId, ((devParam *)(s->EBX))->addr, ((devParam *)(s->EBX))->buf, ((devParam *)(s->EBX))->size);
    }
    else if (s->EAX == 64)
    {
        close(((devParam *)(s->EBX))->typeId, ((devParam *)(s->EBX))->deviceId);
    }
    else if (s->EAX == 65)
    {
        info(((devParam *)(s->EBX))->typeId, ((devParam *)(s->EBX))->deviceId, ((devParam *)(s->EBX))->buf);
    }
    else if (s->EAX == 66)
    {
        *(uint32_t *)(s->ECX) = info_by_index(*(uint32_t *)(s->ECX), s->EBX);
    }
    else if (s->EAX == 100)
    { // 文件系统相关,系统调用
        *(uint32_t *)(s->ECX) = syscall_fs_open(s->EBX);
    }
    else if (s->EAX == 101)
    {
        fp = (file_param *)(s->EBX);
        if (fp != NULL)
        {
            *(uint32_t *)(s->ECX) = syscall_read_fs(fp->fd, fp->addr, fp->buf, fp->size);
        }
    }
    else if (s->EAX == 102)
    {
        fp = (file_param *)(s->EBX);
        if (fp != NULL)
        {
            *(uint32_t *)(s->ECX) = syscall_write_fs(fp->fd, fp->addr, fp->buf, fp->size);
        }
    }
    else if (s->EAX == 103)
    {
        syscall_close_fs(s->EBX);
    }
    else if (s->EAX == 104)
    {
        fp = (file_param *)(s->EBX);
        if (fp != NULL)
        {
            *(bool *)(s->ECX) = make_dir(fp->fd, fp->buf);
        }
    }
    else if (s->EAX == 105)
    {
        fp = (file_param *)(s->EBX);
        if (fp != NULL)
        {
            *(bool *)(s->ECX) = make_file(fp->fd, fp->buf);
        }
    }
    else if (s->EAX == 106)
    {
        // 删除文件,不是文件夹
        fp = (file_param *)(s->EBX);
        if (fp != NULL)
        {
            *(bool *)(s->ECX) = remove_file(fp->fd, fp->buf);
        }
    }
    else if (s->EAX == 107)
    {
        // 删除文件夹
        fp = (file_param *)(s->EBX);
        if (fp != NULL)
        {
            *(bool *)(s->ECX) = remove_dir(fp->fd, fp->buf);
        }
    }
}