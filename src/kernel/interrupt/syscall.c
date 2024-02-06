
#include "interrupt.h"

extern void initSem(uint16_t __value, int32_t *semId);
extern bool sem_open(uint32_t semId);
extern void semWait(int32_t semId);
extern void semSignal(int32_t semId);

extern void yeid();

extern uint32_t open(uint32_t typeId, uint32_t deviceId);
extern int32_t read(uint32_t typeId, uint32_t deviceId, uint32_t addr, char *buf, uint32_t size);
extern int32_t write(uint32_t typeId, uint32_t deviceId, uint32_t addr, char *buf, uint32_t size);
extern uint32_t control(uint32_t typeId, uint32_t deviceId, uint32_t cmd, int32_t *args, uint32_t n);
extern void close(uint32_t typeId, uint32_t deviceId);

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
        s->EIP = s->EIP - 2; // 回到中断调用的eip位置
        semWait(s->EBX);
        break;
    case 53:
        // ebx==>semId
        semSignal(s->EBX);
        break;
    default:
        break;
    }
}