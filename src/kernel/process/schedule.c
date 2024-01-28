#include "schedule.h"
extern processManager manager;
extern TSS *Tss;
extern memoryMarket market;
RBNode nil; // 红黑树的nil节点

char PCBcmp(PCB *pcb_a, PCB *pcb_b)
{
    uint32_t temp;
    if (pcb_a->vruntime >= pcb_b->vruntime)
    {
        temp = pcb_a->vruntime - pcb_b->vruntime;
        if (temp > MAX / 2)
        {
            return -1;
        }
        return 1;
    }

    temp = pcb_b->vruntime - pcb_a->vruntime;
    if (temp > MAX / 2)
    {
        return 1;
    }
    return -1;
}
RBTree *wait;
PCB *findBestProcess()
{
    RBNode *best = delete_min(wait);
    if (best == wait->Nil)
    {
        return NULL;
    }

    return best->data;
}

void insertWait(PCB *pcb)
{
    insert(wait, &(pcb->node));
}

// 至少有一个活跃的进程,不考虑没有进程的情况
void schedule()
{

    PCB *p = manager.now;
    p->runtime++;
    // 判断是否要切换
    if (p->runtime < p->weight)
    {
        return;
    }
    p->vruntime++;
    p->runtime = 0;
    // 找到下一个进程
    manager.now = findBestProcess();
    if (manager.now == NULL)
    {
        manager.now = p;
        return;
    }

    manager.now->status = RUNNING;
    p->status = WAIT;
    insertWait(p);

    // 修改tss 0特权级栈
    update_tss_esp(Tss, manager.now->esp0);

    // 切换页表
    switchUser(market.virMemPool, &(manager.now->u), manager.now->pagePAddr, manager.now->pageVAddr);
}
void initSchedule()
{
    // 初始化wait队列
    initRBTree(&(manager.wait), &nil, PCBcmp);
    wait = &(manager.wait);
}