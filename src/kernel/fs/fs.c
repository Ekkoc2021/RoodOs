// 文件系统
#include "fs.h"

extern device disk_dev[DISKMAXLENGTH];
extern uint32_t diskCount; // 磁盘数量
extern uint32_t mallocPage_u(memoryMarket *market, uint32_t *paddr);
extern uint32_t mallocPage_k(memoryMarket *market, uint32_t *paddr);
extern void freePage(memoryMarket *market, uint32_t vAddr);
extern void tidy(memoryMarket *market);
extern uint32_t mallocMultpage_k(memoryMarket *market, uint32_t n);
extern uint32_t mallocMultpage_u(memoryMarket *market, uint32_t n);
extern void freeNPage(memoryMarket *market, uint32_t vaddr, uint32_t n);

extern memoryMarket market;

// fs内存分配
void malloc_mem(partition *p)
{
    // 根据分区分配bitmap,以及inode缓存节点
    // 当前分配内存最大块为4kb*1024支持最大的内存映射为:4*1024*1024*8*512=16GB
    // 分区最大内存映射最大为16GB
    super_block *super_b = &p->sb;
    uint32_t bytes = super_b->block_bitmap.size % 8 == 0 ? super_b->block_bitmap.size / 8 : super_b->block_bitmap.size / 8 + 1;
    uint32_t block_bitmap_pageSize = bytes % 512 == 0 ? bytes / 512 : bytes / 512 + 1;

    bytes = super_b->inode_bitmap.size % 8 == 0 ? super_b->inode_bitmap.size / 8 : super_b->inode_bitmap.size / 8 + 1;
    uint32_t inode_bitmap_pageSize = bytes % 512 == 0 ? bytes / 512 : bytes / 512 + 1;
    // block_bitmap_pageSize + inode_bitmap_pageSize + OPENINODEPAGESIZE 不大于1024
    // 也就是说分区大小是有上限的,由于连续内存分配问题
    uint32_t mem = mallocMultpage_k(&market, block_bitmap_pageSize + inode_bitmap_pageSize + OPENINODEPAGESIZE);
    super_b->block_bitmap.bits = mem;
    super_b->inode_bitmap.bits = mem + 4096 * block_bitmap_pageSize;

    initCQueue(&p->openInode, mem + 4096 * block_bitmap_pageSize + 4096 * inode_bitmap_pageSize, sizeof(inode), OPENINODESIZE);
}

void save_block_bitmap(partition *p)
{
    disk_dev[p->my_disk->dev_index].type->write(&(disk_dev[p->my_disk->dev_index]),
                                                p->sb.block_bitmap_lba, p->sb.block_bitmap.bits, p->sb.block_bitmap_sects * 512);
}
void load_block_bitmap(partition *p)
{
    disk_dev[p->my_disk->dev_index].type->read(&(disk_dev[p->my_disk->dev_index]),
                                               p->sb.block_bitmap_lba, p->sb.block_bitmap.bits, p->sb.block_bitmap_sects * 512);
}

void save_inode_bitmap(partition *p)
{
    disk_dev[p->my_disk->dev_index].type->write(&(disk_dev[p->my_disk->dev_index]),
                                                p->sb.inode_bitmap_lba, p->sb.inode_bitmap.bits, p->sb.inode_bitmap_sects * 512);
}

void load_inode_bitmap(partition *p)
{
    disk_dev[p->my_disk->dev_index].type->read(&(disk_dev[p->my_disk->dev_index]),
                                               p->sb.inode_bitmap_lba, p->sb.inode_bitmap.bits, p->sb.inode_bitmap_sects * 512);
}

// 保存超级块,以及对应的bitmap
void save_super_block(partition *p)
{
    save_block_bitmap(p);
    save_inode_bitmap(p);
    disk_dev[p->my_disk->dev_index].type->write(&(disk_dev[p->my_disk->dev_index]), p->sb.part_lba_base, &p->sb, sizeof(super_block));
}

// 包装一个读分区的函数
// sec不是分区偏移,而是整个磁盘内的位置
void read_partition(partition *p, char *buff, uint32_t sec, uint32_t size)
{
    disk_dev[p->my_disk->dev_index].type->read(&(disk_dev[p->my_disk->dev_index]), p->start_lba + sec, buff, size);
}

// 包装一个写分区函数
void write_partition(partition *p, char *buff, uint32_t sec, uint32_t size)
{
    disk_dev[p->my_disk->dev_index].type->write(&(disk_dev[p->my_disk->dev_index]), p->start_lba + sec, buff, size);
}

// 加载超级块,同时分配bitmap内存以及加载磁盘bitmap到内存
void load_super_block(partition *p)
{
    super_block *super_b = &p->sb;
    disk_dev[p->my_disk->dev_index].type->read(&(disk_dev[p->my_disk->dev_index]), p->start_lba, super_b, sizeof(super_block));
    if (super_b->magic == SUPERBLOCKMAGIC)
    {
        malloc_mem(p);
        load_block_bitmap(p);
        load_inode_bitmap(p);
    }
}

// 读取inode
inode *load_inode(partition *p, uint32_t index)
{
    // 计算inode偏移扇区
    uint32_t sec = index / 7;
    sec = sec + p->sb.inode_bitmap_lba;
    // 扇区内的偏移位置
    index = index % 7;
    // 缓冲区
    char buff[512];
    read_partition(p, buff, sec, 512);
    inode *temp = buff;
    inode *target;

    // 缓存读取到的inode
    for (uint16_t i = 0; i < 7; i++)
    {
        if (temp[i].i_no < p->sb.inode_cnt && testBit(&p->sb.inode_bitmap, temp[i].i_no))
        {
            // 编号在范围内同时,编号有效
            if (cirIsFull(&p->openInode))
            {
                inode *useless;
                // 当前已经满了,读取当前7个有效扇区保存到
                cirDequeue(&p->openInode, &useless);
            }

            cirEnqueue(&p->openInode, temp + i);
            if (i == index)
            {
                cirRear(&p->openInode, &target);
            }
        }
    }

    // 识别剩下inode是否有效
    return target;
}

// 保存inode到分区
void save_inode(partition *p, inode *i)
{
    // todo
    //  如果该节点在缓冲区中存在,紧挨着的7个元素大概率是同一个扇区的

    // 7个都是同一个扇区的,直接将7个元素,一起保存到inode中

    // 若不是同一个,先读取缺失的内容,再将缓冲区数据并如原来确实数据,再保存
}

// 构建分区
void buildSuperBlock(partition *p, uint32_t inode_cnt)
{
    super_block *super_b = &p->sb;
    super_b->magic = SUPERBLOCKMAGIC;
    super_b->sec_cnt = p->sec_cnt;
    super_b->inode_cnt = inode_cnt;
    super_b->part_lba_base = p->start_lba;

    super_b->block_bitmap_lba = super_b->part_lba_base + 1;
    super_b->block_bitmap_sects = p->sec_cnt % 8 == 0 ? p->sec_cnt / 8 : p->sec_cnt / 8 + 1;
    super_b->block_bitmap.size = super_b->block_bitmap_sects;
    super_b->block_bitmap.used = 0;

    super_b->inode_bitmap_lba = super_b->block_bitmap_lba + super_b->block_bitmap_sects;
    super_b->inode_bitmap_sects = inode_cnt % 8 == 0 ? inode_cnt / 8 : inode_cnt / 8 + 1; // 计算所用字节数量
    super_b->inode_bitmap_sects = super_b->inode_bitmap_sects % 512 == 0 ? super_b->inode_bitmap_sects / 512 : super_b->inode_bitmap_sects / 512 + 1;
    super_b->inode_bitmap.size = inode_cnt;
    super_b->inode_bitmap.used = 0;

    super_b->inode_table_lba = super_b->inode_bitmap_lba + super_b->inode_bitmap_sects;
    super_b->inode_table_sects = inode_cnt % (512 / sizeof(inode)) == 0 ? inode_cnt / (512 / sizeof(inode)) : inode_cnt / (512 / sizeof(inode)) + 1;

    super_b->data_start_lba = super_b->inode_table_lba + super_b->inode_table_sects;
    super_b->root_inode_no = 0;
    super_b->dir_entry_size = sizeof(dir_entry);

    malloc_mem(p);
    // 清理
    clearBitmap(&super_b->inode_bitmap);
    clearBitmap(&super_b->block_bitmap);
    // 设置
    setBit(&super_b->inode_bitmap, 0);
    // 读取inode

    disk_dev[p->my_disk->dev_index].type->write(&(disk_dev[p->my_disk->dev_index]), super_b->part_lba_base, super_b, sizeof(super_block));
}

bool identify_super_b(partition *p)
{
    super_block *super_b = &p->sb;
    load_super_block(p);
    if (super_b->magic == SUPERBLOCKMAGIC)
    {
        return true;
    }
    return false;
}

void fs_init()
{
    log("fs_init %d", sizeof(dir_entry));
    // 解析分区数据,读取分区根目录,查看挂载详情,至少要有分区
    // 检查分区情况
    struct disk *d = disk_dev[0].data;
    // 识别第一个分区的超级块
    if (!identify_super_b(&d->prim_parts[0]))
    {
        // 未成功识别到,构建分区
        buildSuperBlock(&d->prim_parts[0], d->prim_parts[0].sec_cnt / 100 * 5);
    }
    else
    {
        // 分配bitmap内存

        // 读取对应bitmap到指定内存
    }

    // 针对两个
}