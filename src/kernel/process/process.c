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
extern PCB *findBestProcess();
extern void insertWait(PCB *pcb);
extern void schedule();
extern void initSchedule();
TSS *Tss;

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
// 反正也是在内核空间测试进程调度
//
void function()
{
    char buff[128];
    sprintf_(buff, "PID:%d ,name:%s ,vruntime:%d,current tick: %d\n", manager.now->id, manager.now->name, manager.now->vruntime, manager.now->runtime + 1);
    while (1)
    {

        asm volatile(
            "movl %0, %%eax\n "
            "int $0x30\n "
            :
            : "r"(buff));
    }
}
//
PCB *c;
StackInfo *s;
char createProcess(uint16_t pageSize, uint16_t weight, uint16_t argsLength, char *name, ...)
{
    // 创建pcb,读取磁盘,设置esp0
    uint16_t pid = getPID();
    if (pid == 0)
    {
        return false;
    }

    // 创建PCB:pcb页,以及0特权级栈
    PCB *pcb = createPCB(name, pid, weight);
    usePID(pcb, pid);
    c = pcb;
    // 初始化0特权级栈
    // StackInfo *s; todo..
    s = (StackInfo *)(pcb->esp0);

    // 切换到用户页表项
    switchUser(market.virMemPool, &pcb->u, pcb->pagePAddr, pcb->pageVAddr);

    // -----根据name加载用户程序 todo 加载用户程序,并设置-------------------
    uint32_t paddr;
    uint32_t me = mallocMultpage_u(&market, pageSize + 1);

    s->EIP = function;

    // 构建用户上下文
    s->IVN = 32;
    s->EFLAGS = (EFLAGS_IOPL_0 | EFLAGS_MBS | EFLAGS_IF_1);

    s->CS = 0b11011;
    s->oldSS = 0b100011;
    s->oldESP = (me) + 4096 * (pageSize + 1) - 32 - argsLength * sizeof(char) - 4;

    //------todo 正确性有待商榷
    // 压入命令行参数
    // 如果是函数,进入默认是通过call进入(考虑压入eip),而不是直接跳转,如果是有带头汇编一起编译,则不需要考虑
    va_list args;               // 定义参数
    va_start(args, argsLength); // 初始化
    memcpy_(s->oldESP + 4, name, argsLength);

    va_end(args);
    //-----------------------------

    s->GS = 0b100011;
    s->FS = 0b100011;
    s->ES = 0b100011;
    s->DS = 0b100011;

    //------------------------------todo----------------------
    // 设置pcb为就绪
    pcb->status = WAIT;
    // 往就绪队列中加入该pcb
    pcb->node.data = pcb;
    insertWait(pcb);

    // 切回原用户
    //-------------------------------------------------------------------
    switchUser(market.virMemPool, &(manager.now->u), manager.now->pagePAddr, manager.now->pageVAddr);
}

// 进程模块
void initProcess(TSS *tss, GDT *gdt)
{

    Tss = tss;
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

    //-------测试-----

    char buff[50];
    for (uint16_t i = 1; i < 40; i++)
    {
        sprintf_(buff, "task%d", i);

        createProcess(1, i % 5, strlen_(buff), buff);
    }

    manager.now = findBestProcess();
    switchUser(market.virMemPool, &manager.now->u, manager.now->pagePAddr, manager.now->pageVAddr);

    // (*functionPtr)();
    // iretd_function();
    // __asm__("jmp dword 0b11011:0" ::);
    update_tss_esp(tss, manager.now->esp0);
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
    // ----------------
    // iretd_function();
}
