#include "process.h"

#define EFLAGS_MBS (1 << 1)     // 此项必须要设置
#define EFLAGS_IF_1 (1 << 9)    // if为1,开中断
#define EFLAGS_IF_0 0           // if为0,关中断
#define EFLAGS_IOPL_3 (3 << 12) // IOPL3,用于测试用户程序在非系统调用下进行IO
#define EFLAGS_IOPL_0 (0 << 12) // IOPL0

#define USER_CODE_SELECTOR (3 << 3 | 0b11)
#define USER_DATA_SELECTOR (4 << 3 | 0b11)

#define INITPCB 0xc0007000

processManager manager;
extern memoryMarket market;
extern kernel roodos;
extern PCB *theNextProcess();
extern void insertWait(PCB *pcb);
extern void schedule();
extern void initSchedule();
extern void removeProcess(PCB *pcb);
extern void yeid();
extern TSS *Tss;

// 为pcb分配内存
void initPCB(PCB *pcb, char *name, uint16_t id)
{
}
// 解析某个elf文件后,将程序展开后放入内存中,然后调用createProcess创建一个进程
// 调用createProcess处于内核态,分配好pcb后,直接且先切换到pcb对应的页表,根据elf头文件数据将数据读出,然后写入
// 有时候程序也不一定是从磁盘读出:就简单的认为程序就是只从磁盘启动
// 用户指定了一个路径,内核拿到路径,解析后,按照解析结果读取放到用户空间对应的位置,然后调用createProcess创建进程

PCB *createPCB(char *name, uint32_t id, uint16_t weight)
{
    // 分配内存
    uint32_t paddr;
    uint32_t pcbm = mallocPage_k(&market, &paddr);
    if (pcbm == 0)
    {
        return 0; // 分配pcb的内存都不够
    }

    // 创建pcb
    uint32_t s0 = mallocPage_k(&market, &paddr);
    if (s0 == 0)
    {
        freePage(&market, pcbm);
        return 0; // 分配0特权级栈内存不够
    }

    // 初始化名称
    PCB *pcb = (PCB *)pcbm;
    for (uint16_t i = 0; i < 127; i++)
    {
        if (name[i] == '\0')
        {
            pcb->name[i] = '\0';
            break;
        }
        pcb->name[i] = name[i];
    }

    pcb->status = CREATE;
    pcb->weight = weight;
    pcb->runtime = 0;

    // 初始化
    pcb->id = id;
    pcb->vruntime = 0;
    pcb->esp0 = s0 + 4080 - sizeof(StackInfo); // 留一点间隙,计算不好,刚刚好4095,容易犯错

    uint32_t pageVaddr = mallocPage_k(&market, &paddr);
    if (pageVaddr == 0)
    {
        freePage(&market, pcbm);
        freePage(&market, s0);
        return 0;
    }

    pcb->pageVAddr = pageVaddr;
    pcb->pagePAddr = paddr;
    initUserPd((&market)->virMemPool, pageVaddr);
    initBitmap2(&(pcb->u.map), 768, pcb->u.bit);

    // 初始化pd
    for (uint16_t i = 0; i < 768; i++)
    {
        pcb->u.PT[i] = -1;
    }
    pcb->father = manager.now;
    initLinkedList(&(pcb->children));
    add(&(manager.now->children), &(pcb->tag));
    return pcb;
}

void loaderUserProgram(PCB *userPCB)
{
}
uint16_t getPID()
{
    for (uint16_t i = 1; i < TASKSIZE; i++)
    {
        if (manager.task[i] == 0)
        {
            return i;
        }
    }
    return 0;
}

void usePID(uint32_t pcbAddr, uint16_t pid)
{
    manager.task[pid] = pcbAddr;
}

void initStack(StackInfo *s, uint32_t eip, uint32_t esp3)
{
    s->EIP = eip;
    s->IVN = 32;
    s->EFLAGS = (EFLAGS_IOPL_0 | EFLAGS_MBS | EFLAGS_IF_1);
    s->CS = 0b11011;
    s->oldSS = 0b100011;
    s->oldESP = esp3;
    s->GS = 0b100011;
    s->FS = 0b100011;
    s->ES = 0b100011;
    s->DS = 0b100011;
}
// 反正也是在内核空间测试进程调度
void function()
{
    char buff[128];
    sprintf_(buff, "PID:%d ,name:%s ,vruntime:%d,current tick: %d\n", manager.now->id, manager.now->name, manager.now->vruntime, manager.now->runtime + 1);
    while (1)
    {
        __asm__ __volatile__(
            "movl $1, %%eax\n"
            "movl %0, %%ebx\n"
            "int $0x30\n"
            :
            : "r"(buff)
            : "%eax", "%ebx");

        __asm__ __volatile__(
            "movl $11, %%eax\n"
            "int $0x30\n"
            :
            :
            : "%eax");
    }
}

/*
    获取可用PID
    创建PCB:分配PCB内存,分配0特权级栈内存
    切换到对应PCB页表,解析用户程序头确定分配内存,展开用户程序到指定内存
    初始化0特权级栈:EIP
*/
uint16_t createProcess(uint16_t weight, uint16_t argsLength, char *name, ...)
{
    // 创建pcb,读取磁盘,设置esp0
    uint16_t pid = getPID();
    if (pid == 0)
    {
        return 0;
    }

    // 创建PCB:pcb页,以及0特权级栈
    PCB *pcb = createPCB(name, pid, weight);
    usePID(pcb, pid);

    // 切换到用户页表项
    switchUser(market.virMemPool, &pcb->u, pcb->pagePAddr, pcb->pageVAddr);

    /*
        todo 24 1 31:
        根据名称在磁盘加载用户程序到内存,这里不涉及加载
        直接将内核某个函数设置为用户程序,同时直接分配内存
    */
    uint32_t paddr;
    uint32_t me = mallocMultpage_u(&market, 1);
    StackInfo *s = (StackInfo *)(pcb->esp0);
    // 初始化0特权级栈
    initStack(s, function, (me) + 4096 * (1) - 32 - argsLength * sizeof(char) - 4);

    //------todo 24 1 31 -------
    /*
        正确性有待具体测试,目前看来缺陷很大
        根据传入参数个数,将数据压入用户栈中
    */
    va_list args;               // 定义参数
    va_start(args, argsLength); // 初始化
    memcpy_(s->oldESP + 4, name, argsLength);
    va_end(args);
    //-----------------------------

    // 设置pcb为就绪
    pcb->status = WAIT;
    // 往就绪队列中加入该pcb
    insertWait(pcb);

    // 切回原用户
    switchUser(market.virMemPool, &(manager.now->u), manager.now->pagePAddr, manager.now->pageVAddr);
    return pid;
}

// 将某个pcb占用的内存回收,其他进程对某个进程pcb的回收
void destroyPCB(PCB *pcb)
{
    // 是不是应该检查一下删除的pcb是否有效?

    // 从对应的度队列中删除pcb
    // 现在只有等待队列 todo : 需要考虑其他队列的情况
    removeProcess(pcb);

    // 在父亲节点中删除pcb
    PCB *f = (PCB *)(pcb->father);
    deleteNode(&(f->children), &(pcb->tag));

    // 任务列表中删除
    manager.task[pcb->id] = 0;

    // 处理其子进程,合并到init线程
    mergeList(&(manager.init->children), &(pcb->children));
    /*首先回收用户内存
        // 搜索用户非空页表项
        // 找到非空页表项,释放其页表项的页目录中物理内存
        // 释放页目录项对应内存
    */

    switchUser(market.virMemPool, &(pcb->u), pcb->pagePAddr, pcb->pageVAddr);
    userPageDir *u = &(pcb->u);
    uint32_t PTPaddr;
    uint32_t temp;
    uint16_t count;
    for (uint16_t i = 0; i < 768; i++)
    {
        if (u->PT[i] >= 0)
        {

            // 获取物理页
            PTPaddr = getPDE(market.virMemPool, i) >> 12 << 12;
            // 说明分配有内存,至少分配有页目录
            if (u->PT[i] > 0)
            {
                // 清理页
                count = 0;
                for (uint16_t j = 0; j < 1024; j++)
                {

                    temp = getPTE(PTPaddr, j) & 0xFFFFF000;
                    if (temp != 0)
                    {
                        ReturnPhyPage(market.phyPool, temp);
                        count++;
                    }
                    if (count == u->PT[i])
                    {
                        break;
                    }
                }
            }
            // 归还
            ReturnPhyPage(market.phyPool, PTPaddr);
        }
    }

    switchUser(market.virMemPool, &manager.now->u, manager.now->pagePAddr, manager.now->pageVAddr);
    // 回收页表内存
    freePage(&market, pcb->pageVAddr);
    // 回收esp0内存
    freePage(&market, pcb->esp0);
    // 回收pcb占用内存
    freePage(&market, pcb);
}

// 进程拷贝

// pcb共享==>线程

// 进程模块

// 切换到3特权级,在等待队列中至少一个用户程序
void switch_to_user_mode()
{
    // 找到一个可运行的进程,切换到对应进程==>切换到3特权级
    manager.now = theNextProcess();
    switchUser(market.virMemPool, &manager.now->u, manager.now->pagePAddr, manager.now->pageVAddr);
    update_tss_esp(Tss, manager.now->esp0);
    asm volatile(
        "movl %0, %%eax\n "
        "movl %%eax, %%esp\n "
        :
        : "r"(manager.now->esp0) // 输入操作数：myVar表示源操作数
    );
    __asm__("xchg %%bx,%%bx" ::);
    asm volatile(
        "jmp intr_exit\n" // 将myVar的值赋给eax寄存器
    );
}
void initProcess(TSS *tss, GDT *gdt)
{
    initTss(tss, gdt);

    // 初始化调度器
    initSchedule();

    // 初始化init线程,设置pcb,永远不会调用
    PCB *initP = (PCB *)INITPCB;
    initP->id = 0;
    manager.task[0] = initP;
    initP->pagePAddr = market.virMemPool->paddr;
    initP->pageVAddr = market.virMemPool->vaddr;
    memcpy_(&(initP->u), market.virMemPool->userPD, sizeof(userPageDir));
    manager.now = initP;
    manager.init = initP;
    initLinkedList(&(manager.now->children));
}
