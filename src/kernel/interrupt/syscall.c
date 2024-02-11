
#include "interrupt.h"

extern void initSem(uint16_t __value, int32_t *semId);
extern bool sem_open(uint32_t semId);
extern void semWait(int32_t semId);
extern void semSignal(int32_t semId);

extern void yeid();

typedef struct
{
    char *buf;
    uint32_t typeId;
    uint32_t deviceId;
    uint32_t addr;
    uint32_t size;
} devParam;

extern uint32_t open(uint32_t typeId, uint32_t deviceId);
extern int32_t read(uint32_t typeId, uint32_t deviceId, uint32_t addr, char *buf, uint32_t size);
extern int32_t write(uint32_t typeId, uint32_t deviceId, uint32_t addr, char *buf, uint32_t size);
extern uint32_t control(uint32_t typeId, uint32_t deviceId, uint32_t cmd, int32_t *args, uint32_t n);
extern void close(uint32_t typeId, uint32_t deviceId);
extern void info(uint32_t typeId, uint32_t deviceId, char *buff);
extern int32_t info_by_index(int32_t index, char *buff);

void sys_call(StackInfo *s)
{
    switch (s->EAX)
    {
    case 1: // 输出中断
        log_qemu_printf(s->EBX);
        break;
        // case 10: // 创建进程
        //     log_qemu_printf(s->EBX);
        // break;
    case 11: // yeid,主动让出cpu
        yeid();
        break;
    case 50: // 信号量初始化
        // ebx==>value
        // ecx==>*semId
        initSem(s->EBX, s->ECX);
        break;
    case 51:
        // ebx==>semId
        sem_open(s->EBX);
        break;
    case 52:
        // ebx==>semId
        semWait(s->EBX);
        break;
    case 53:
        // ebx==>semId
        semSignal(s->EBX);
        break;
    case 60: // 设备管理相关系统调用
             // ebx==>devParam
             // ecx==>返回参数地址
        *(uint32_t *)(s->ECX) = open(((devParam *)(s->EBX))->typeId, ((devParam *)(s->EBX))->deviceId);
        break;
    case 61:
        // ebx==>devParam
        // ecx==>返回参数地址
        *(uint32_t *)(s->ECX) = read(((devParam *)(s->EBX))->typeId, ((devParam *)(s->EBX))->deviceId, ((devParam *)(s->EBX))->addr, ((devParam *)(s->EBX))->buf, ((devParam *)(s->EBX))->size);
        break;
    case 62:
        // ebx==>devParam
        // ecx==>返回参数地址
        *(uint32_t *)(s->ECX) = write(((devParam *)(s->EBX))->typeId, ((devParam *)(s->EBX))->deviceId, ((devParam *)(s->EBX))->addr, ((devParam *)(s->EBX))->buf, ((devParam *)(s->EBX))->size);
        break;
    case 63:
        // *(uint32_t *)(s->ECX) = write(devPar->typeId, devPar->deviceId, devPar->addr, devPar->buf, devPar->size);
        break;
    case 64:
        // ebx==>devParam
        // ecx==>返回参数地址
        close(((devParam *)(s->EBX))->typeId, ((devParam *)(s->EBX))->deviceId);
        break;
    case 65:
        // ebx==>devParam
        // ecx==>返回参数地址
        info(((devParam *)(s->EBX))->typeId, ((devParam *)(s->EBX))->deviceId, ((devParam *)(s->EBX))->buf);
        break;
    case 66:
        // ebx=int32_t* index
        // ecx=char *buff
        *(uint32_t *)(s->ECX) = info_by_index(*(uint32_t *)(s->ECX), s->EBX);
        break;
    default:
        break;
    }
}