/*
 * @Author: EKKO 189890049@qq.com
 * @Date: 2024-01-13 21:21:27
 * @LastEditors: EKKO 189890049@qq.com
 * @LastEditTime: 2024-01-13 22:11:15
 * @Description:
 */

#include "physicalMem.h"

// #define IGNORE1MBMEM

// 根据传入数据进行初始化
memPool phyPool;
void showIndexNode(indexNode *tempP)
{
    log("- indexBitsAddr:0x%p,indexOfBitMap:%d,size:%d,used:%d\n", tempP->indexBit.bits, tempP->startIndexOfBitmap, tempP->indexBit.size, tempP->indexBit.used);
}
void showPool(memPool *p)
{
    log("---physical memory report---\n");
    log(" page size:%d,used:%d  ", p->map.size, p->map.used);
    char buff[128];
    log("   Block of available memory:%d\n", p->e->size);
    for (uint16_t i = 0; i < p->e->size; i++)
    {
        iToHexStr((p->e->memBlocks[i]).addr, buff);
        log("+ addr: 0x%s\\%d ", buff, (p->e->memBlocks[i]).addr);
        log(" memory size: %d B ", p->e->memBlocks[i].memSize);
        log(" bitsize: %d \n", p->e->memBlocks[i].bitSize);
        log("  - bitIndex between %d and %d \n", p->e->memBlocks[i].bitMinIndex, p->e->memBlocks[i].bitMaxIndex);
    }
    log("   indexLisnt:elementSize: %d length :%d dataAddr: 0x%p \n", p->indexList.elemSize, p->indexList.length, p->indexList.data);
    indexNode *tempP;
    for (uint32_t i = 0; i < p->indexList.length; i++)
    {
        getElem(&(p->indexList), &tempP, i);
        log("+ data details  addr:0x%p\n    ",
            (indexNode *)(p->indexList.data) + i);
        showIndexNode(tempP);
    }
}
// e820map调出可用内存,然后构建memPool
memPool *initPhysicalMem(e820map *e)
{
    log("-- Physical memory initialization... --\n");
    phyPool.map.bits = (char *)(START + OFFSET);
    phyPool.e = (availableMem *)(e->entries + e->length);
    phyPool.e->size = 0;
    phyPool.e->memBlocks = (block *)(phyPool.e + 1);
    uint32_t meCount = 0;
    log("Block of memory:%d\n", e->length);
    uint16_t availableCount = 0;
    uint32_t totalBitSize = 0;
    char buff[128];
    for (uint16_t i = 0; i < e->length; i++)
    {
        iToHexStr((e->entries[i]).addr, buff);
        log("  addr: 0x%s\\%d ", buff, (e->entries[i]).addr);
        log("  size: %d B ", (e->entries[i]).size);
        log("  type: %d \n", (e->entries[i]).type);

        if (e->entries[i].type == 1)
        {

#ifdef IGNORE1MBMEM
            if (e->entries[i].addr < 0x100000)
            {
                log("  memory with address of  0x%s\\%d"
                    " and size of %d B is discarded\n",
                    buff, (uint32_t)(e->entries[i]).addr, (e->entries[i]).size);

                e->entries[i].type = 2;
                continue;
            }
#endif
            // 处理起始地址不为4096整数倍的情况
            // 实际上不会有这种情况
            uint16_t m = e->entries[i].addr % 4096;
            if (m != 0)
            {
                e->entries[i].addr = (e->entries[i].addr + 4096) / 4096;
                e->entries[i].size = e->entries[i].size - (4096 - m);
            }

            phyPool.e->size++;
            phyPool.e->memBlocks[phyPool.e->size - 1].addr = e->entries[i].addr;
            phyPool.e->memBlocks[phyPool.e->size - 1].memSize = e->entries[i].size & 0xFFFFF000;
            phyPool.e->memBlocks[phyPool.e->size - 1].bitSize = phyPool.e->memBlocks[phyPool.e->size - 1].memSize / 4096;
            phyPool.e->memBlocks[phyPool.e->size - 1].bitMinIndex = totalBitSize;
            totalBitSize = phyPool.e->memBlocks[phyPool.e->size - 1].bitSize + totalBitSize;
            phyPool.e->memBlocks[phyPool.e->size - 1].bitMaxIndex = totalBitSize - 1;
            meCount = meCount + e->entries[i].size;
        }
    }

    // 按照可用块来处理
    log("  total available memory : %d Byte ,total bitmap size : %d\n", meCount, totalBitSize);

    phyPool.map.size = totalBitSize;
    phyPool.map.used = 0; // 后续处理

    // 初始化indexList
    phyPool.indexList.elemSize = sizeof(indexNode);
    // 计算总bitmap之后的地址,将其视作,indexList.data的起始位置 ,是否需要考虑位置对齐?
    uint32_t bitsSize = totalBitSize % 8 == 0 ? totalBitSize / 8 : totalBitSize / 8 + 1;
    uint32_t targetAddr = (uint32_t)(phyPool.map.bits + bitsSize);
    phyPool.indexList.data = targetAddr;

    // 初始化 phyPool.indexList.data
    phyPool.indexList.length = 0;
    indexNode temp;
    for (uint32_t i = 0; i < bitsSize / 1024; i++)
    {
        initBitmap2(&(temp.indexBit), 1024 * 8, phyPool.map.bits + 1024 * i);
        temp.startIndexOfBitmap = 1024 * 8 * i;
        phyPool.indexList.length++;
        setElem(&(phyPool.indexList), &(temp), i);

        /* code */
    }

    if (bitsSize % 1024 != 0)
    {
        temp.startIndexOfBitmap = 1024 * 8 * phyPool.indexList.length;
        initBitmap2(&(temp.indexBit), (bitsSize % 1024) * 8, phyPool.map.bits + 1024 * phyPool.indexList.length);
        phyPool.indexList.length++;
        setElem(&(phyPool.indexList), &(temp), phyPool.indexList.length - 1);
    }

    // showPool(&phyPool);
    log("-- Physical memory Initialization successful! --\n");
    return &phyPool;
}

uint32_t initKernelPhysicalMem(memPool *phyPool)
{
    // 初始化内核占用物理内存
    uint32_t maxVaddr = (uint32_t)(((block *)phyPool->indexList.data) + phyPool->indexList.length);
    uint32_t pageSize = (maxVaddr - START) % 4096 == 0 ? (maxVaddr - START) / 4096 : (maxVaddr - START) / 4096 + 1;
    log("size of the kernel: %d Page \n", pageSize);

    // 定位到某个block
    indexNode *INP;
    uint32_t indexOfBlock = phyAddrToMemBlocks(phyPool, maxVaddr - START);
    uint32_t indexOfIndexList = phyAddrToIndexList(phyPool, phyPool->e->memBlocks[indexOfBlock].addr);
    getElem(&(phyPool->indexList), &INP, indexOfIndexList);
    // 内核不会超过1mb内存,不考虑超过了1mb内存的情况
    for (uint32_t i = 0; i < pageSize; i++)
    {
        setBit(&(INP->indexBit), i);
        phyPool->map.used++;
    }
    return 4096 * pageSize;
}

char compareTo(void *A, void *B)
{
    indexNode *a = (indexNode *)A;
    indexNode *b = (indexNode *)B;
    if ((a->indexBit.size - a->indexBit.used) > (b->indexBit.size - b->indexBit.used))
    {
        return 1;
    }
    else if ((a->indexBit.size - a->indexBit.used) < (b->indexBit.size - b->indexBit.used))
    {
        return -1;
    }
    return 0;
}

uint32_t phyAddrToIndexList(memPool *p, uint32_t bitmapIndex)
{
    block *b = p->e->memBlocks;
    for (uint32_t i = 0; i < p->e->size; i++)
    {
        if (b[i].bitMinIndex <= bitmapIndex && bitmapIndex <= b[i].bitMaxIndex)
        {
            return i;
        }
    }
    return p->e->size;
}

uint32_t getPhyPage(memPool *p)
{
    // showPool(p);
    arrayList *list = &(p->indexList);
    // 在indexList中找到空位最多的块
    uint32_t index = getByCmp(list, compareTo);

    // 拿到最多的块
    indexNode *temp;
    getElem(list, &temp, index);
    // log("the max empty in index list : %d \n start of the index %d \n", index, temp->startIndexOfBitmap);
    // 判断容量最多的块的容量是否足够
    if (temp->indexBit.size - temp->indexBit.used == 0)
    {
        return NULL;
    }
    // 足够
    Bitmap *bitmap = &(temp->indexBit);
    uint32_t indexOfBitmap = find_fist_bit(bitmap) + temp->startIndexOfBitmap; // 在全局的索引
    // log("index of bitmap %d \n", indexOfBitmap);
    setBit(bitmap, indexOfBitmap - temp->startIndexOfBitmap);
    p->map.used++;

    // 从index找到对应的大物理块
    uint32_t indexOfBlock = phyAddrToIndexList(p, indexOfBitmap);
    // log("the max index of memory block : %d", indexOfBlock);
    // log("block %d,indexOfBitmap %d,indexOfBlock %d \n", index, indexOfBitmap, indexOfBlock);
    // log("p mem :%p \n", p->e->memBlocks[indexOfBlock].addr + (indexOfBitmap - p->e->memBlocks[indexOfBlock].bitMinIndex) * 4096);
    return p->e->memBlocks[indexOfBlock].addr + (indexOfBitmap - p->e->memBlocks[indexOfBlock].bitMinIndex) * 4096;
}

uint32_t phyAddrToMemBlocks(memPool *p, uint32_t phyAdd)
{
    block *b = p->e->memBlocks;
    for (uint32_t i = 0; i < p->e->size; i++)
    {
        if (b[i].addr <= phyAdd && phyAdd < (b[i].addr + b[i].memSize))
        {
            return i;
        }
    }
    return p->e->size;
}

uint32_t ReturnPhyPage(memPool *p, uint32_t phyAddr)
{
    uint32_t pageAddr = phyAddr & 0xFFFFF000;
    // 查找在那个大物理块
    uint32_t indexOfblock = phyAddrToMemBlocks(p, pageAddr);
    // 从大物理块中推导出在那个索引表中
    uint32_t indexOfBitmap = (pageAddr - p->e->memBlocks[indexOfblock].addr) / 4096 + p->e->memBlocks[indexOfblock].bitMinIndex;
    // 从索引表清除位
    // 找到对应索引表
    uint32_t indexOfIndexList = indexOfBitmap / (8 * 1024);
    Bitmap *b = &((indexNode *)(p->indexList.data))[indexOfIndexList].indexBit;
    clearBit(b, indexOfBitmap - p->e->memBlocks[indexOfblock].bitMinIndex);
    p->map.used--;
    // log("free 0x%p,indexOfBlock %d,indexOfBitmap %d,indexofIndexList %d\n", pageAddr, indexOfblock, indexOfBitmap, indexOfIndexList);
    // 修改总bitmap的used数据
}