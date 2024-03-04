
#include "memory.h"

memoryMarket market;
e820map map;
userPageDir initUPD;

void memdebug(memoryMarket *market)
{
    showPool(market->phyPool);
    logVir(market->virMemPool);
}
// 内核页目录项全映射
uint16_t kernelPDEMapping(memoryMarket *market)
{
    uint32_t phyPage;
    for (uint16_t indexOfPD = 1; indexOfPD < 256; indexOfPD++)
    {

        uint32_t phyPage1 = getPhyPage(market->phyPool);
        if (phyPage1 == 0x0)
        {
            // 内存不够分配新页表
            ReturnPhyPage(market->phyPool, phyPage);
            return 0;
        }
        clearPage(phyPage1); // 清除一个页
        setPDE(market->virMemPool, phyPage1 | PAGEATTR, indexOfPD + 768);
        // 设置PD的余量
        resetPTSize(market->virMemPool, indexOfPD + 768, 0);
    }
    return 1;
}
memoryMarket *initMemoryManagement(uint32_t size, void *addr, void *kernelVAddr, void *pTablePhAddr)
{
    // 初始化 页部件

    // 查看内存管理
    map.entries = (e820entry *)addr;
    map.length = size;

    log("start Init memory management module...\n");
    log("   pageTable physical addr : 0x%p \n", pTablePhAddr);
    log("   pageTable virtual addr : 0x%p \n", ((uint32_t)pTablePhAddr + (uint32_t)kernelVAddr));
    char buff[128];
    // sprintf_(buff, "Block of available memory:%d\n", map.length);
    // print(buff);
    memPool *PhyPool = initPhysicalMem(&map);
    log("-- start init virtual memory... --\n");
    virtualMemPool *virPoor = initPageDir(((uint32_t)pTablePhAddr + (uint32_t)kernelVAddr), pTablePhAddr, &initUPD);
    market.initPageDir = &initPageDir;
    market.phyPool = PhyPool;
    market.virMemPool = virPoor;

    // showPool(PhyPool);
    // // 物理内存去除kernel占用部分
    log("   init kernel virtual memory\n");
    uint32_t kernelSize = initKernelPhysicalMem(PhyPool);
    log("-- init virtual memory successful! --\n");
    log("   The kernel uses memory size: %f KB! \n", kernelSize / 4096.0);

    // // 虚拟内存去除kernel占用部分
    uint16_t indexOfKernelPDE = START >> 22;
    log("   the index of kernel pde : %d \n", indexOfKernelPDE);
    log("   start init kernel virtual memory \n ");
    initkernelVirturalMem(virPoor, kernelSize / 4096, indexOfKernelPDE);
    log("   kernel virtual memory init done \n");
    log("-- start kernelPDEMapping , size: 256 * 4 KB ! --\n");
    ASSERT(kernelPDEMapping(&market)); // 内存不够就映射失败!
    log("--memory module init successful! --\n");
    showPool(PhyPool);

    // 24/3/4 内存分配存在严重问题,导致整个系统崩溃
    uint32_t p_t;
    p_t = mallocPage_k(&market, &p_t);
    return &market;
}

uint32_t mallocPage_u(memoryMarket *market, uint32_t *paddr)
{
    // 防止中断嵌套
    char status = BeSureDisable_irq();
    uint32_t phyPage = getPhyPage(market->phyPool);
    *paddr = phyPage;
    if (phyPage == 0x0)
    {
        return 0x0;
    }
    // 获取或配置虚拟页
    int16_t indexOfPD = searchUserPD(market->virMemPool);
    if (indexOfPD == 768)
    {
        // 没有任何空页表项,需要分配新页表
        indexOfPD = searchUserEmptyPD(market->virMemPool);

        if (indexOfPD == -1)
        {
            // 虚拟页用满了
            ReturnPhyPage(market->phyPool, phyPage);
            return 0x0;
        }

        uint32_t phyPage1 = getPhyPage(market->phyPool);
        if (phyPage1 == 0x0)
        {
            // 内存不够分配新页表
            ReturnPhyPage(market->phyPool, phyPage);
            return 0x0;
        }

        clearPage(phyPage1); // 清除一个页
        setPDE(market->virMemPool, phyPage1 | PAGEATTR, indexOfPD);
        // 设置PD的余量
        resetPTSize(market->virMemPool, indexOfPD, 0);
    }

    // 拿到页表物理地址
    uint32_t phyPTaddr = getPDE(market->virMemPool, indexOfPD) >> 12 << 12;

    // 搜索空页表项
    uint16_t indexOfPT = searchPTE(phyPTaddr, 0x0);
    if (indexOfPT > 1023)
    {
        ReturnPhyPage(market->phyPool, phyPage);
        return 0x0; // 按道理永远不会走这里
    }

    // 设置页表项
    setPTE(phyPTaddr, phyPage | PAGEATTR, indexOfPT);
    // 更新PD余量
    updatePTSzie(market->virMemPool, indexOfPD, 1);
    // 构造设置好的页的虚拟地址
    uint32_t vAddr = (indexOfPD << 22) + (indexOfPT << 12);
    invlpg(vAddr);

    Resume_irq(status);
    return vAddr;
}

uint32_t mallocPage_k(memoryMarket *market, uint32_t *paddr)
{
    char status = BeSureDisable_irq();
    // 分配物理页
    uint32_t phyPage = getPhyPage(market->phyPool);

    *paddr = phyPage;
    if (phyPage == 0x0)
    {
        return 0x0;
    }

    // 获取或配置虚拟页
    int16_t indexOfPD = searchKernelPD(market->virMemPool);
    if (indexOfPD == 1024)
    {
        // 没有任何空页表项,需要分配新页表
        indexOfPD = searchKernelEmptyPD(market->virMemPool);
        if (indexOfPD == -1)
        {
            // 虚拟页用满了
            ReturnPhyPage(market->phyPool, phyPage);
            return 0x0;
        }

        uint32_t phyPage1 = getPhyPage(market->phyPool);
        if (phyPage1 == 0x0)
        {
            // 内存不够分配新页表
            ReturnPhyPage(market->phyPool, phyPage);
            return 0x0;
        }
        clearPage(phyPage1); // 清除一个页
        setPDE(market->virMemPool, phyPage1 | PAGEATTR, indexOfPD);
        // 设置PD的余量
        resetPTSize(market->virMemPool, indexOfPD, 0);
    }

    // 拿到页表物理地址
    uint32_t phyPTaddr = getPDE(market->virMemPool, indexOfPD) >> 12 << 12;

    // 搜索空页表项
    uint16_t indexOfPT = searchPTE(phyPTaddr, 0x0);
    if (indexOfPT > 1023)
    {
        ReturnPhyPage(market->phyPool, phyPage);
        return 0x0; // 按道理永远不会走这里
    }

    // 设置页表项
    setPTE(phyPTaddr, phyPage | PAGEATTR, indexOfPT);
    // 更新PD余量
    updatePTSzie(market->virMemPool, indexOfPD, 1);
    // 构造设置好的页的虚拟地址
    uint32_t vAddr = (indexOfPD << 22) + (indexOfPT << 12);
    invlpg(vAddr);
    Resume_irq(status);
    return vAddr;
}

void freePage(memoryMarket *market, uint32_t vAddr)
{
    char status = BeSureDisable_irq();
    // 提取 PDE的index
    uint32_t indexOfPD = vAddr >> 22;
    uint32_t indexOfPT = (vAddr >> 12) & 0b1111111111;

    // 获得对应PT的物理地址
    uint32_t PTPAddr = getPDE(market->virMemPool, indexOfPD) & (~PAGEATTR);

    // 获得对应PTE包含的物理地址
    uint32_t paddr = getPTE(PTPAddr, indexOfPT) & (~PAGEATTR);
    ReturnPhyPage(market->phyPool, paddr);

    // 设置对应PTE条目
    setPTE(PTPAddr, 0x0, indexOfPT);

    // 更新PD
    updatePTSzie(market->virMemPool, indexOfPD, -1);
    invlpg(vAddr);
    Resume_irq(status);
}

// 将PD为0的物理页归还
void tidy(memoryMarket *market)
{
    // 不整理内核,内核要保证全映射
    uint16_t index;
    uint32_t PTPaddr;

    // 整理用户
    while ((index = getUserZeroPdIndex(market->virMemPool)) < 768)
    {
        // 获取物理页
        PTPaddr = getPDE(market->virMemPool, index) >> 12 << 12;
        // 归还
        ReturnPhyPage(market->phyPool, PTPaddr);
        // 重置PDsize
        resetPTSize(market->virMemPool, index, -1);
        // 清理
        clearPDE(market->virMemPool, index);
    }
}

// 没考虑到连续多页内存的情况,补充两个函数
// 实现一个简单函数:假设用户申请连续内存超过4*1024Kb的情况
// todo : 考虑超过4mb的情况
uint32_t mallocMultpage_k(memoryMarket *market, uint32_t n)
{
    char status = BeSureDisable_irq();
    // 找到一个空的pd
    // 没有任何空页表项,需要分配新页表
    uint32_t indexOfPD;
    // int16_t serchZeroSizeKernelPD(virtualMemPool * p)
    indexOfPD = searchZeroSizeKernelPD(market->virMemPool);
    if (indexOfPD == 1024)
    {
        // todo 不应该就怎么草率处理
        return 0x0;
    }

    // 拿到页表物理地址
    uint32_t phyPage;
    uint32_t phyPTaddr = getPDE(market->virMemPool, indexOfPD) >> 12 << 12;
    for (uint32_t i = 0; i < n; i++)
    {
        // 分配物理页
        phyPage = getPhyPage(market->phyPool);
        // 设置页表项
        setPTE(phyPTaddr, phyPage | PAGEATTR, i);
    }

    // 更新PD余量
    updatePTSzie(market->virMemPool, indexOfPD, n);
    Resume_irq(status);
    return (indexOfPD << 22) + (0 << 12);
}
uint32_t mallocMultpage_u(memoryMarket *market, uint32_t n)
{
    char status = BeSureDisable_irq();
    // 找到一个空的pd
    // 没有任何空页表项,需要分配新页表
    uint32_t indexOfPD;

    indexOfPD = searchUserEmptyPD(market->virMemPool);
    if (indexOfPD == -1)
    {
        return 0x0;
    }
    uint32_t phyPage = getPhyPage(market->phyPool);
    if (phyPage == 0x0)
    {
        return 0x0;
    }
    // 检查物理内存是否足够
    if (market->phyPool->map.size - market->phyPool->map.used < n)
    {
        ReturnPhyPage(market->phyPool, phyPage);
        return 0x0;
    }
    clearPage(phyPage); // 清除一个页
    setPDE(market->virMemPool, phyPage | PAGEATTR, indexOfPD);
    // 设置PD的余量
    resetPTSize(market->virMemPool, indexOfPD, 0);

    // 根据需要分配内存

    // 拿到页表物理地址
    uint32_t phyPTaddr = getPDE(market->virMemPool, indexOfPD) >> 12 << 12;
    for (uint32_t i = 0; i < n; i++)
    {
        // 分配物理页
        phyPage = getPhyPage(market->phyPool);
        // 设置页表项
        setPTE(phyPTaddr, phyPage | PAGEATTR, i);
    }
    // 更新PD余量
    updatePTSzie(market->virMemPool, indexOfPD, n);
    Resume_irq(status);
    return (indexOfPD << 22) + (0 << 12);
}

void freeNPage(memoryMarket *market, uint32_t vaddr, uint32_t n)
{
    char status = BeSureDisable_irq();
    for (uint32_t i = 0; i < n; i++)
    {
        freePage(market, vaddr);
        vaddr += 4096;
    }
    Resume_irq(status);
}

//------------------ todo : 细粒度内存管理---------------
// 指定内存页将其拆分为指定大小的内存区域

void page_split(uint32_t addr, uint32_t size)
{
}

listNode *searchUsefulNode(linkedList *mem_List)
{
    listNode *temp = mem_List->head.next;
    memoryArea *ma;
    while (temp != &mem_List->head)
    {
        ma = (memoryArea *)(temp->data);
        if (ma->bitmap.used < ma->bitmap.size)
        {
            return temp;
        }

        temp = temp->next;
    }
    return NULL; // 不属于内核的区域
}
listNode *getUserfulNodeBySzie(memoryManager *m, uint32_t size)
{
    listNode *temp;
    if (size <= 16)
    {
        temp = searchUsefulNode(&m->byte_16_list);
    }
    else if (size <= 32)
    {
        temp = searchUsefulNode(&m->byte_32_list);
    }
    else if (size < 64)
    {
        temp = searchUsefulNode(&m->byte_32_list);
    }
    else if (size <= 128)
    {
        temp = searchUsefulNode(&m->byte_128_list);
    }
    else if (size <= 256)
    {
        temp = searchUsefulNode(&m->byte_256_list);
    }
    else if (size <= 512)
    {
        temp = searchUsefulNode(&m->byte_512_list);
    }
    else if (size <= 1024)
    {
        temp = searchUsefulNode(&m->byte_1024_list);
    }
}
void *sys_malloc(memoryManager *m, uint32_t size)
{

    if (size > 1024 && size <= 4096)
    {
        return mallocPage_k(&market, &size);
    }
    else if (size > 4096)
    {
        uint32_t page = size % 4096 == 0 ? size / 4096 : size / 4096 + 1;
        return mallocMultpage_k(&market, page);
    }

    listNode *temp;
    temp = getUserfulNodeBySzie(m, size);
    if (temp == NULL)
    {
        /* code */
    }
}