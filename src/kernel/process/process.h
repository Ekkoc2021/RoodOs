#ifndef PROCESS_H_12
#define PROCESS_H_12
#include "../include/memory.h"
#include "tss.h"
#include "../include/global.h"
#include "../include/RBTree.h"
#define TASKSIZE 256 // 最多支持256个任务

typedef enum
{
    CREATE = 1,
    RUNNING,
    BLOCKED,
    WAIT,
    END
} status;

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
    userPageDir u;       // 多大?至少3000B
    // 父亲进程链表
    uint32_t father;
    // 孩子进程
    uint32_t children[64]; // 最多64个
    RBNode node;
} PCB;

typedef struct
{
    PCB *now;    // 正在运行的进程
    PCB *next;   //
    RBTree wait; // 等待中的队列
    uint32_t task[TASKSIZE];
} processManager;
void initProcess(TSS *tss, GDT *gdt);
void schedule();
PCB *createPCB(char *name, uint32_t id, uint16_t weight);
#endif