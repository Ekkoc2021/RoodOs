#include "virtualMem.h"

void logVir(pageDir *pd)
{
    // 如何遍历一个pd对象?
    // index PDE PTSzie
    uint32_t ptphy;
    // 内核
    // uint16_t debug = 230;
    for (uint16_t i = 0; i < 256; i++)
    {

        if (testBit(&(pd->kernelPD->map), i))
        {
            ptphy = (getPDE(pd, 768 + i) >> 12) << 12;
            log("index:%d,PDE:0x%p,PTSzie:%d \n", i, getPDE(pd, 768 + i), getPTSize(pd, 768 + i));
            uint16_t size = 1;

            for (uint32_t j = 0; j < 1024; j++)
            {

                uint32_t pte = getPTE(ptphy, j);
                if (pte != 0)
                {
                    size++;
                    log(" %d :0x%p ", j, pte);
                    if (size % 5 == 0)
                    {
                        log("\n");
                    }
                }
            }
        }
    }

    log("\n");
    // 用户
    for (uint16_t i = 0; i < 768; i++)
    {
        ptphy = (getPDE(pd, i) >> 12) << 12;
        if (testBit(&(pd->userPD->map), i))
        {
            log("index:%d,PDE:0x%p,PTSzie:%d \n", i, getPDE(pd, i), getPTSize(pd, i));
            uint16_t size = 1;
            for (uint32_t j = 0; j < 1024; j++)
            {

                uint32_t pte = getPTE(ptphy, j);
                if (pte != 0)
                {
                    size++;
                    log(" %d :%p ", j, pte);
                    if (size % 4 == 0)
                    {
                        log("\n");
                    }
                }
            }
        }
    }
    return;
}
uint32_t getPTSize(pageDir *p, uint16_t index)
{
    uint16_t *v = p->kernelPD->PT;
    if (index > 767)
    {
        return v[index - 768];
    }
    v = p->userPD->PT;
    return v[index];
}
void initkernelVirturalMem(pageDir *pd, uint32_t pageSize, uint16_t PDEIndex)
{
    // 同样只考虑1mb以内
    // 根据PEDindex得到页表物理地址
    uint32_t paddr = (getPDE(pd, PDEIndex) >> 12) << 12;
    // 根据页表物理地址重新设置页表项不在内核范围内虚拟PTE
    for (uint16_t i = pageSize; i < 256; i++)
    {
        setPTE(paddr, 0x0, i);
    }

    // 初始化 PTsize ,没有设置物理页全部为-1
    for (uint16_t i = 0; i < 1024; i++)
    {
        resetPTSize(pd, i, -1);
    }

    // 更新PDsize
    resetPTSize(pd, PDEIndex, pageSize + 1);
    // 设置临时页
    clearTemp(); // 主动设置为#FFFF
    // 更新pd的bitmap
    setBit(&(pd->kernelPD->map), PDEIndex - 768);
}
// 刷新虚拟地址
void invlpg(void *virtualMem)
{
    asm volatile("invlpg (%0)" ::"r"(virtualMem) : "memory");
}
// 将一个物理页映射到临时区域,方便操作
void setTemp(uint32_t paddr)
{
    uint32_t tempPTE = (paddr & 0xFFFFF000) | 0b11; // 构造临时PTE
    uint32_t *vpt = TEMP_PTVADDR;                   // 获取临时页表
    vpt[TEMP_PTE] = tempPTE;                        // 设置临时页表项
    invlpg(((TEMP_PDE << 22) + (TEMP_PTE << 12)));  // 刷新临时虚拟内存
}

// 清空临时区域
void clearTemp()
{
    // 感觉不用清除也可以!这个位置永远只有内核能访问到!
    uint32_t *vpt = TEMP_PTVADDR;
    vpt[TEMP_PTE] = 0xFFFF;
    // invlpg((void *)((TEMP_PDE << 22) + (TEMP_PTE << 12)));
}

// 清理一个物理页
void clearPage(uint32_t paddr)
{
    // 这个时候给物理内存,是无法定位的
    // 先借助内核已经映射的页表,将要清空物理地址暂时映射到可定位空虚拟页目录内
    setTemp(paddr);
    char *target = (char *)((TEMP_PDE << 22) + (TEMP_PTE << 12)); // 拿到要清空页
    memset_(target, 0, 4096);
    clearTemp(); // 清空
}

// 设置页表的页表项
void setPTE(uint32_t PTPAddr, uint32_t PTE, uint16_t index)
{

    setTemp(PTPAddr);
    uint32_t *target = (TEMP_PDE << 22) + (TEMP_PTE << 12);
    target[index] = PTE;
    clearTemp();
    // paddr是要映射的物理页
}

// 设置一个页目录项
void setPDE(pageDir *p, uint32_t PDE, uint16_t index)
{
    uint32_t *v = p->vaddr;
    v[index] = PDE;
    if (index > 767)
    {
        setBit(&(p->kernelPD->map), index - 768);
        return;
    }
    setBit(&(p->userPD->map), index);
}
void clearPDE(pageDir *p, uint16_t index)
{
    uint32_t *v = p->vaddr;
    v[index] = 0x0;
    if (index > 767)
    {
        clearBit(&(p->kernelPD->map), index - 768);
        p->kernelPD->PT[index - 768] = -1;
        return;
    }
    clearBit(&(p->userPD->map), index);
    p->userPD->PT[index] = -1;
}
uint32_t getPTE(uint32_t PTPAddr, uint16_t index)
{

    setTemp(PTPAddr);
    uint32_t *target = (TEMP_PDE << 22) + (TEMP_PTE << 12);
    uint32_t pte = target[index];
    clearTemp();
    return pte;
    // paddr是要映射的物理页
}

void updatePTSzie(pageDir *p, uint16_t pdeIndex, int16_t increment)
{
    if (pdeIndex > 767)
    {
        // 内核
        p->kernelPD->PT[pdeIndex - 768] += increment;
        /* code */
        return;
    }
    p->userPD->PT[pdeIndex] += increment;
}

void resetPTSize(pageDir *p, uint16_t pdeIndex, int16_t value)
{
    if (pdeIndex > 767)
    {
        // 内核
        p->kernelPD->PT[pdeIndex - 768] = value;
        return;
    }
    p->userPD->PT[pdeIndex] = value;
}

uint32_t getPDE(pageDir *p, uint16_t index)
{
    uint32_t *v = p->vaddr;
    return v[index];
}

uint16_t searchPTE(uint32_t PTPAddr, uint32_t PTE)
{
    setTemp(PTPAddr);
    uint32_t *target = (uint32_t *)((TEMP_PDE << 22) + (TEMP_PTE << 12));
    for (uint16_t i = 0; i < 1024; i++)
    {
        if (target[i] == PTE)
        {
            return i;
        }
    }
    clearTemp();
    return 1024;
}
pageDir pd;
KernelPageDir pkd;
//-----------pageEntry management---------------------
pageDir *initPageDir(void *vaddr, void *paddr, userPageDir *upd)
{

    pd.vaddr = vaddr;
    pd.paddr = paddr;
    pd.kernelPD = &pkd;
    pd.userPD = upd;
    initBitmap2(&(pd.kernelPD->map), 256, pd.kernelPD->bit);
    initBitmap2(&(pd.userPD->map), 768, pd.userPD->bit);
    return &pd;
}

//--------------Kernel virtual pageDir management------------
// 初始化一个内核部分page

// 在内核页目录查找一个空目录项,小于768为没有空页目录项
int16_t searchKernelEmptyPD(pageDir *p)
{
    return find_fist_bit(&(p->kernelPD->map)) + 768;
}

// 查找一个有余量的内核目录项
int16_t searchKernelPD(pageDir *p)
{
    for (uint16_t i = 0; i < 256; i++)
    {
        if ((p->kernelPD->PT)[i] >= 0 && (p->kernelPD->PT)[i] < 1024)
        {
            return i + 768;
        }
    }
    return 1024;
}

int16_t nextKernelPDZeroIndex = 0;
// 寻找内核为余量为0的目录项
int16_t getKernelZeroPdIndex(pageDir *p)
{
    for (; nextKernelPDZeroIndex < 256; nextKernelPDZeroIndex++)
    {
        if ((p->kernelPD->PT)[nextKernelPDZeroIndex] == 0)
        {
            return nextKernelPDZeroIndex + 768;
        }
    }
    if (nextKernelPDZeroIndex == 256)
    {
        nextKernelPDZeroIndex = 0;
    }

    return 1024;
}

//------------user virtual pageDir management---------

// 在用户页目录查找一个空目录项
int16_t searchUserEmptyPD(pageDir *p)
{
    return find_fist_bit(&(p->userPD->map));
}

// 查找一个有余量的目录项
int16_t searchUserPD(pageDir *p)
{
    for (uint16_t i = 0; i < 768; i++)
    {
        if ((p->userPD->PT)[i] >= 0 && (p->userPD->PT)[i] < 1024)
        {
            return i;
        }
    }
    return 768;
}
int16_t nextUserPDZeroIndex = 0;
int16_t getUserZeroPdIndex(pageDir *p)
{
    for (; nextUserPDZeroIndex < 768; nextUserPDZeroIndex++)
    {
        if ((p->userPD->PT)[nextUserPDZeroIndex] == 0)
        {
            return nextUserPDZeroIndex;
        }
    }
    if (nextUserPDZeroIndex == 768)
    {
        nextUserPDZeroIndex = 0;
    }

    return 768;
}