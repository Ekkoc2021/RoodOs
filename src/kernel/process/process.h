#ifndef PROCESS_H_12
#define PROCESS_H_12
#include "../include/memory.h"
#include "tss.h"
#include "../include/global.h"
#include "../include/RBTree.h"
#include "../include/linkedList.h"
#include "../include/linkedQueue.h"
#define TASKSIZE 256 // 最多支持256个任务

typedef enum
{
    CREATE = 1,
    RUNNING,
    BLOCKED,
    WAIT,
    SLEEP,
    END
} status;

typedef struct
{
    uint32_t ino;
    uint32_t file_type;
} file_descriptors;

typedef struct
{
    char name[128]; // 都是从磁盘加载,一般是磁盘名称
    uint16_t status;
    uint16_t weight;  // 默认值为 CREATE
    uint16_t runtime; // runtime增加 weight时增加一个vruntime
    uint32_t id;
    uint32_t vruntime;
    uint32_t esp0;
    uint32_t stack3Szie; // 3特权级栈大小,默认1kb
    uint32_t esp3;       // 最开始就是程序的入口地址
    uint32_t pageVAddr;  // 页表虚拟地址
    uint32_t pagePAddr;  // 页表物理地址
    uint32_t code;       // 返回值
    uint32_t father;     // 父进程
    userPageDir u;       // 多大?至少3000B
    // 父亲进程:当pre为NULL且
    linkedList children;
    listNode tag;       // 孩子节点链表节点标签
    queueNode blockTag; // 阻塞队列链表节点标签
    RBNode node;
    bool justWakeUp; // 是否是刚刚重睡眠状态回到运行态
    file_descriptors *file_descriptors;
} PCB;

typedef struct
{
    PCB *now;    // 正在运行的进程
    PCB *init;   //
    RBTree wait; // 等待中的队列
    uint32_t task[TASKSIZE];
    uint32_t minVruntime;
    uint32_t ticks; // 共运行了多少个tick
} processManager;

void initProcess(TSS *tss, GDT *gdt);
void schedule();
PCB *createPCB(char *name, uint32_t id, uint16_t weight);

#endif