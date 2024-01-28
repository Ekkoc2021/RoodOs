#ifndef VIRTUALMEM_H
#define VIRTUALMEM_H
#include "../include/types.h"
#include "../include/bitmap.h"
#include "../include/str.h"
#include "../include/roodos.h"
#define PAGEATTR 0b111
#define TEMP_PTE 0b1111111111
#define TEMP_PDE 0b1100000000
#define TEMP_PTVADDR 0xc0003000

typedef struct
{
    /* data */
    char bit[32];
    Bitmap map;
    int16_t PT[256]; // 大于等于0有效
} KernelPageDir;
// typedef struct
// {
//     unsigned char *bits; // 1设置,0为没有设置
//     bMLengthType used;   // 已经设置长度
//     bMLengthType size;   // 尚未设置的长度,位域的数量
// } Bitmap;
typedef struct
{
    char bit[128 - 32];
    Bitmap map;
    int16_t PT[768];
} userPageDir;
typedef struct
{
    unint32 paddr;  // pd的物理地址
    uint32_t vaddr; // pd的虚拟地址
    KernelPageDir *kernelPD;
    userPageDir *userPD;
} pageDir, virtualMemPool;

// 刷新虚拟地址
void invlpg(void *virtualMem);
// 辅助函数
void setTemp(uint32_t paddr);
void clearTemp();
// 清空一个物理页
void clearPage(uint32_t paddr);
// 给出一个PTE条目,并设置到对应页表中,页表给的是物理地址
void setPTE(uint32_t PTPAddr, uint32_t PTE, uint16_t index);

uint32_t getPTE(uint32_t PTPAddr, uint16_t index);

// 给一个ptp的地址,在pd中搜索,并返回其index
uint16_t searchPTE(uint32_t PTPAddr, uint32_t PTE);

// 给出一个PDE条目,设置并PDE
void setPDE(virtualMemPool *p, uint32_t PDE, uint16_t index);

void clearPDE(virtualMemPool *p, uint16_t index);
// 根据index获取对应PDE条目
uint32_t getPDE(virtualMemPool *p, uint16_t index);

// 根据给出的pdeIndex 跟新对应的PD数值:PD数值+increment,0<=increment<=1024
void updatePTSzie(virtualMemPool *p, uint16_t pdeIndex, int16_t increment);
// 将PDSize值设置为value
void resetPTSize(virtualMemPool *p, uint16_t pdeIndex, int16_t value);
// 初始化页目录:请提前设置pageDir.PT
virtualMemPool *initPageDir(void *vaddr, void *paddr, userPageDir *upd);

// 查找一个0占用的内核页目录项
int16_t searchZeroSizeKernelPD(virtualMemPool *p);

// 查找一个内核空页目录返回索引,小于768为没有空页目录项
int16_t searchKernelEmptyPD(virtualMemPool *p);
// 查找一个页表有余量的内核页目录项,小于768为没有空页目录项
int16_t searchKernelPD(virtualMemPool *p);
int16_t searchUserPD(virtualMemPool *p);

// 1024为没有0容量页
int16_t getKernelZeroPdIndex(virtualMemPool *p);

// ... -1为空
int16_t searchUserEmptyPD(virtualMemPool *p);
// ... -1为空
int16_t searchUserPD(virtualMemPool *p);
// 768为没有0容量页
int16_t getUserZeroPdIndex(virtualMemPool *p);

// 获取容纳量
uint32_t getPTSize(virtualMemPool *p, uint16_t index);
// 输出虚拟内存信息
void logVir(virtualMemPool *pd);
// 初始化内核虚拟内存
void initkernelVirturalMem(virtualMemPool *pd, uint32_t pageSize, uint16_t PDEIndex);

void initUserPd(virtualMemPool *pd, uint32_t PageVaddr);

// 切换页
// void switch_page_dir(uint32_t paddr);

// 切换到用户
void switchUser(virtualMemPool *pd, userPageDir *upd, uint32_t paddr, uint32_t vaddr);

#endif