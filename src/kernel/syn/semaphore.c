#include "semaphore.h"
extern void blockProcess(linkedQueue *blockQueue);
extern void insertWait(PCB *pcb);
extern processManager manager;
extern void backInt30();
sem_t sems[SEMSIZE];

uint32_t seId;
void initSemaphoreMoudle()
{
    for (uint16_t i = 0; i < SEMSIZE; i++)
    {
        sems[i].pcb = 0;
        initQueue(&(sems[i].block));
    }
    sems[0].pcb = 202432;
}

// 在系统内拿到一个可用的信号量

int32_t getAvailableSem()
{
    for (uint16_t i = 0; i < SEMSIZE; i++)
    {
        if (sems[i].pcb == 0)

        {
            return i;
        }
    }
    return -1;
}

// 通过信号量id,设置信号量

void setSem(uint32_t pcb, uint16_t __value, uint16_t id)
{
    sems[id].pcb = pcb;
    sems[id].value = __value;
}

// 初始化信号量
void initSem(uint16_t __value, int32_t *semId)
{
    uint32_t id = getAvailableSem();
    if (id == -1)
    {
        *semId = -1;
        return;
    }

    setSem(manager.now, __value, id);
    *semId = id;
}
// 打开一个信号量
bool sem_open(uint32_t semId)
{
    if (semId >= SEMSIZE)
    {
        return false;
    }

    return sems[semId]._pshared;
}

// 使用信号量,走系统调用
void semWait(int32_t semId)
{
    // 检查资源量
    if (sems[semId].value <= 0)
    {
        backInt30(); // 重新进行系统调用
        blockProcess(&(sems[semId].block));
    }
    sems[semId].value--;
}

// 给内核用的,没有通过系统调用进来,给内核用的semWait接口
void sys_semWait(int32_t semId)
{
    // 检查资源量
    while (sems[semId].value <= 0)
    {
        // 放开中断
        manager.now->runtime = manager.now->weight - 1;
        enable_irq();
    }
    disable_irq();
    sems[semId].value--;
}

// 归还资源
void semSignal(int32_t semId)
{
    sems[semId].value++;
    queueNode *node = dequeue(&(sems[semId].block));
    if (node != &(sems[semId].block.head))
    {
        // 放入等待队列
        PCB *p = node->data;
        p->vruntime = manager.minVruntime - 1;
        p->runtime = p->weight - 1;
        insertWait(node->data);
        // 唤醒阻塞中的一个进程
    }
}
