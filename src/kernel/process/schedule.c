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
PCB *theNextProcess()
{
    RBNode *best = delete_min(wait);
    if (best == wait->Nil)
    {
        return manager.init;
    }
    return best->data;
}

void removeProcess(PCB *pcb)
{
    RBNode *n = deleteNodeRBT(wait, &(pcb->node));
}

void insertWait(PCB *pcb)
{
    pcb->node.data = pcb;
    insertRBT(wait, &(pcb->node));
}
void switchProcess()
{

    // asm volatile("xchgw %bx, %bx");
    // 切换栈
    asm volatile(
        "movl %0, %%eax\n "
        "movl %%eax, %%esp\n "
        "jmp intr_exit\n"
        :
        : "r"((manager.now->esp0) - sizeof(StackInfo)) // 输入操作数：myVar表示源操作数
    );
}
// 至少有一个活跃的进程,不考虑没有进程的情况
void schedule()
{
    manager.ticks++;

    PCB *p = manager.now;
    p->runtime++;
    // 判断是否要切换
    if (p->runtime <= p->weight)
    {
        return;
    }
    p->vruntime++;
    p->runtime = 0;
    p->status = WAIT;

    insertWait(p);
    // 找到下一个进程
    manager.now = theNextProcess();
    manager.minVruntime = manager.now->vruntime;
    manager.now->status = RUNNING;
    // 修改tss 0特权级栈
    update_tss_esp(Tss, manager.now->esp0);
    // 切换页表
    switchUserPage(market.virMemPool, &(manager.now->u), manager.now->pagePAddr, manager.now->pageVAddr);

    switchProcess();
}

// 当前正在运行的进程主动让出cpu
void yeid()
{
    PCB *p = manager.now;
    p->vruntime++;
    p->runtime = 0;
    p->status = WAIT;
    // 找到下一个进程
    switchProcess();
}

void initSchedule()
{
    // 初始化wait队列
    initRBTree(&(manager.wait), &nil, PCBcmp);
    wait = &(manager.wait);
}