
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

extern memoryMarket market;
extern uint32_t mallocMultpage_u(memoryMarket *market, uint32_t n);

extern uint16_t createProcess3(uint16_t weight, char *exec_file_path, uint16_t argsLength, ...);
typedef struct
{
    uint16_t weight;
    uint16_t argsLength;
    char *exec_file_path;
    char *args;
} proc_info;

void sys_call(StackInfo *s)
{
    file_param *fp;
    if (s->EAX == 1)
    { // 输出中断
        log(s->EBX);
    }
    else if (s->EAX == 11)
    { // yeid,主动让出cpu
        yeid();
    }
    else if (s->EAX == 12)
    {
        exit_pro();
    }
    else if (s->EAX == 13)
    {
        *((uint16_t *)(s->ECX)) = createProcess3(((proc_info *)(s->EBX))->weight, ((proc_info *)(s->EBX))->exec_file_path, ((proc_info *)(s->EBX))->argsLength);
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
    else if (s->EAX == 108)
    {
        // 删除文件夹
        fp = (file_param *)(s->EBX);
        if (fp != NULL)
        {
            *(unsigned int *)(s->ECX) = get_file_type(fp->fd);
        }
    }
    else if (s->EAX == 108)
    {
        // 返回文件大小
        fp = (file_param *)(s->EBX);
        if (fp != NULL)
        {
            *(unsigned int *)(s->ECX) = get_file_size(fp->fd);
        }
    }
    else if (s->EAX == 200)
    {
        // 内存分配系统调用
        *(unsigned int *)(s->ECX) = mallocMultpage_u(&market, s->EBX);
    }
}