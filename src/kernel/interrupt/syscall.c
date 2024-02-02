
#include "interrupt.h"
extern void initSem(uint16_t __value, int32_t *semId);
extern bool sem_open(uint32_t semId);
extern void semWait(int32_t semId);
extern void semSignal(int32_t semId);
extern void yeid();
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
    default:
        break;
    }
}