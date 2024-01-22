
#include "memory.h"

memoryMarket market;
e820map map;
userPageDir initUPD;

memoryMarket *initMemoryManagement(uint32_t size, void *addr, void *kernelVAddr, void *pTablePhAddr)
{
    // 初始化 页部件

    // 查看内存管理
    map.entries = (e820entry *)addr;
    map.length = size;

    log("Init memory management module...\n");
    log("pageTable physical addr : 0x%p \n", pTablePhAddr);
    log("pageTable virtual addr : 0x%p \n", ((uint32_t)pTablePhAddr + (uint32_t)kernelVAddr));
    char buff[128];
    // sprintf_(buff, "Block of available memory:%d\n", map.length);
    // print(buff);
    memPool *PhyPool = initPhysicalMem(&map);
    pageDir *pd = initPageDir(((uint32_t)pTablePhAddr + (uint32_t)kernelVAddr), pTablePhAddr, &initUPD);
    market.initPageDir = &initPageDir;
    market.phyPool = PhyPool;
    market.pagedir = pd;

    // showPool(PhyPool);
    // // 物理内存去除kernel占用部分
    uint32_t kernelSize = initKernelPhysicalMem(PhyPool);
    log("The kernel uses memory size: %f KB! \n", kernelSize / 4096.0);
    showPool(PhyPool);
    // // 虚拟内存去除kernel占用部分
    uint16_t indexOfKernelPDE = START >> 22;
    log("the index of kernel pde : %d \n", indexOfKernelPDE);
    initkernelVirturalMem(pd, kernelSize / 4096, indexOfKernelPDE);
    // logVir(pd);
    //---------------测试-------------------
    // log("\n");
    // uint32_t pt = mallocPage_u(&market);
    // // freePage(&market, pt);
    // // pt = mallocPage_u(&market);
    // // logVir(pd);
    // // showPool(PhyPool);
    // freePage(&market, pt);
    // // showPool(PhyPool);
    // // logVir(pd);
    // pt = mallocPage_k(&market);
    // logVir(pd);
    // showPool(PhyPool);
    // freePage(&market, pt);
    // showPool(PhyPool);
    // logVir(pd);
    // pt = mallocPage_k(&market);
    // logVir(pd);
    // showPool(PhyPool);
    // freePage(&market, pt);
    // showPool(PhyPool);
    // logVir(pd);
    // tidy(&market);
    // showPool(PhyPool);
    // logVir(pd);
    // void *m = mallocPage_k(&market);
    // char *t = m;
    // for (uint32_t i = 0; i < 30; i++)
    // {
    //     t[i] = 'm';
    // }
    // t[30] = '\0';
    // printf(t);
    // freePage(&market, t);
    // printf(t);
    //--------------------------------------

    return &market;
}
uint32_t mallocPage_u(memoryMarket *market)
{
    disable_irq();
    uint32_t phyPage = getPhyPage(market->phyPool);
    if (phyPage == 0x0)
    {
        return 0x0;
    }
    // 获取或配置虚拟页
    int16_t indexOfPD = searchUserPD(market->pagedir);
    if (indexOfPD == 768)
    {
        // 没有任何空页表项,需要分配新页表
        indexOfPD = searchUserEmptyPD(market->pagedir);

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
        setPDE(market->pagedir, phyPage1 | 0x11, indexOfPD);
        // 设置PD的余量
        resetPTSize(market->pagedir, indexOfPD, 0);
    }

    // 拿到页表物理地址
    uint32_t phyPTaddr = getPDE(market->pagedir, indexOfPD) >> 12 << 12;

    // 搜索空页表项
    uint16_t indexOfPT = searchPTE(phyPTaddr, 0x0);
    if (indexOfPT > 1023)
    {
        ReturnPhyPage(market->phyPool, phyPage);
        return 0x0; // 按道理永远不会走这里
    }

    // 设置页表项
    setPTE(phyPTaddr, phyPage | 0x11, indexOfPT);
    // 更新PD余量
    updatePTSzie(market->pagedir, indexOfPD, 1);
    enable_irq();
    // 构造设置好的页的虚拟地址
    uint32_t vAddr = (indexOfPD << 22) + (indexOfPT << 12);
    invlpg(vAddr);
    enable_irq();
    return vAddr;
}

uint32_t mallocPage_k(memoryMarket *market)
{
    disable_irq();
    // 分配物理页
    uint32_t phyPage = getPhyPage(market->phyPool);
    if (phyPage == 0x0)
    {
        return 0x0;
    }

    // 获取或配置虚拟页
    int16_t indexOfPD = searchKernelPD(market->pagedir);
    if (indexOfPD == 1024)
    {
        // 没有任何空页表项,需要分配新页表
        indexOfPD = searchKernelEmptyPD(market->pagedir);
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
        setPDE(market->pagedir, phyPage1 | 0x11, indexOfPD);
        // 设置PD的余量
        resetPTSize(market->pagedir, indexOfPD, 0);
    }

    // 拿到页表物理地址
    uint32_t phyPTaddr = getPDE(market->pagedir, indexOfPD) >> 12 << 12;

    // 搜索空页表项
    uint16_t indexOfPT = searchPTE(phyPTaddr, 0x0);
    if (indexOfPT > 1023)
    {
        ReturnPhyPage(market->phyPool, phyPage);
        return 0x0; // 按道理永远不会走这里
    }

    // 设置页表项
    setPTE(phyPTaddr, phyPage | 0x11, indexOfPT);
    // 更新PD余量
    updatePTSzie(market->pagedir, indexOfPD, 1);
    // 构造设置好的页的虚拟地址
    uint32_t vAddr = (indexOfPD << 22) + (indexOfPT << 12);
    invlpg(vAddr);
    enable_irq();
    return vAddr;
}

void freePage(memoryMarket *market, uint32_t vAddr)
{
    disable_irq();
    // 提取 PDE的index
    uint32_t indexOfPD = vAddr >> 22;
    uint32_t indexOfPT = (vAddr >> 12) & 0b1111111111;

    // 获得对应PT的物理地址
    uint32_t PTPAddr = getPDE(market->pagedir, indexOfPD) & (~0x11);

    // 获得对应PTE包含的物理地址
    uint32_t paddr = getPTE(PTPAddr, indexOfPT) & (~0x11);

    ReturnPhyPage(market->phyPool, paddr);

    // 设置对应PTE条目
    setPTE(PTPAddr, 0x0, indexOfPT);

    // 更新PD
    updatePTSzie(market->pagedir, indexOfPD, -1);
    invlpg(vAddr);
    enable_irq();
}

// 将PD为0的物理页归还
void tidy(memoryMarket *market)
{
    // 整理内核
    uint16_t index;
    uint32_t PTPaddr;
    while ((index = getKernelZeroPdIndex(market->pagedir)) < 1024)
    {
        // 获取物理页
        PTPaddr = getPDE(market->pagedir, index) >> 12 << 12;
        // 归还
        ReturnPhyPage(market->phyPool, PTPaddr);
        // 重置PDsize
        resetPTSize(market->pagedir, index, -1);
        // 清理
        clearPDE(market->pagedir, index);
    }

    // 整理用户
    while ((index = getUserZeroPdIndex(market->pagedir)) < 768)
    {
        // 获取物理页
        PTPaddr = getPDE(market->pagedir, index) >> 12 << 12;
        // 归还
        ReturnPhyPage(market->phyPool, PTPaddr);
        // 重置PDsize
        resetPTSize(market->pagedir, index, -1);
        // 清理
        clearPDE(market->pagedir, index);
    }
}