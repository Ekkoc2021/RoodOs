#include "process.h"
#include "../include/memory.h"
#define CREATE 0
#define RUNNING 1
#define BLOCKED 2
#define WAIT 3
#define END 4
typedef struct
{
    char name[128];
    uint16_t status;
    uint16_t weight;  // 默认值为
    uint16_t runtime; // runtime增加 weight时增加一个vruntime
    uint32_t id;
    uint32_t vruntime;
    uint32_t esp0;
    uint32_t ThreeStackSize; // 3特权级栈大小,默认1kb
    uint32_t esp3;
    uint32_t pageAddr; // 页表地址

} PCB;

void initPCB()
{
}
// 进程模块
void initProcess(TSS *tss)
{
    initTss(tss);
}

//
