#include "fs.h"

extern device disk_dev[DISKMAXLENGTH];

extern memoryMarket market;

void initInodeBuff(partition *p)
{
    inode *buff = p->openInode.data.data;
    for (uint32_t i = 0; i < OPENINODESIZE; i++)
    {
        buff[i].i_no = p->sb.inode_cnt;
    }
}

// sec不是分区偏移,而是整个磁盘内的位置
void read_partition(partition *p, char *buff, uint32_t sec, uint32_t size)
{
    disk_dev[p->my_disk->dev_index].type->read(&(disk_dev[p->my_disk->dev_index]), sec, buff, size);
}

// 包装一个写分区函数
void write_partition(partition *p, char *buff, uint32_t sec, uint32_t size)
{
    disk_dev[p->my_disk->dev_index].type->write(&(disk_dev[p->my_disk->dev_index]), sec, buff, size);
}

// fs内存分配
void malloc_mem(partition *p)
{
    // 根据分区分配bitmap,以及inode缓存节点
    // 当前分配内存最大块为4kb*1024支持最大的内存映射为:4*1024*1024*8*512=16GB
    // 分区最大内存映射最大为16GB
    super_block *super_b = &p->sb;
    uint32_t bytes = super_b->block_bitmap.size % 8 == 0 ? super_b->block_bitmap.size / 8 : super_b->block_bitmap.size / 8 + 1;
    uint32_t block_bitmap_pageSize = bytes % 4096 == 0 ? bytes / 4096 : bytes / 4096 + 1;

    bytes = super_b->inode_bitmap.size % 8 == 0 ? super_b->inode_bitmap.size / 8 : super_b->inode_bitmap.size / 8 + 1;
    uint32_t inode_bitmap_pageSize = bytes % 4096 == 0 ? bytes / 4096 : bytes / 4096 + 1;
    // block_bitmap_pageSize + inode_bitmap_pageSize + OPENINODEPAGESIZE 不大于1024
    // 也就是说分区大小是有上限的,由于连续内存分配问题
    uint32_t mem = mallocMultpage_k(&market, block_bitmap_pageSize + inode_bitmap_pageSize + OPENINODEPAGESIZE);
    super_b->block_bitmap.bits = mem;
    super_b->inode_bitmap.bits = mem + 4096 * block_bitmap_pageSize;

    initCQueue(&p->openInode, mem + 4096 * block_bitmap_pageSize + 4096 * inode_bitmap_pageSize, sizeof(inode), OPENINODESIZE);
    inode *temp = p->openInode.data.data;
    for (uint32_t i = 0; i < OPENINODESIZE; i++)
    {
        temp[i].i_no = p->sb.inode_cnt + p->sb.inode_global_start_index + 1;
    }
}

void save_block_bitmap(partition *p)
{
    uint32_t size = p->sb.block_bitmap.size % 8 == 0 ? p->sb.block_bitmap.size / 8 : p->sb.block_bitmap.size / 8 + 1;
    write_partition(p, p->sb.block_bitmap.bits, p->sb.block_bitmap_lba, size);
}

void load_block_bitmap(partition *p)
{
    uint32_t size = p->sb.block_bitmap.size % 8 == 0 ? p->sb.block_bitmap.size / 8 : p->sb.block_bitmap.size / 8 + 1;
    read_partition(p, p->sb.block_bitmap.bits, p->sb.block_bitmap_lba, size);
}

void save_inode_bitmap(partition *p)
{
    uint32_t size = p->sb.inode_bitmap.size % 8 == 0 ? p->sb.inode_bitmap.size / 8 : p->sb.inode_bitmap.size / 8 + 1;

    write_partition(p, p->sb.inode_bitmap.bits, p->sb.inode_bitmap_lba, size);
}

void load_inode_bitmap(partition *p)
{
    uint32_t size = p->sb.inode_bitmap.size % 8 == 0 ? p->sb.inode_bitmap.size / 8 : p->sb.inode_bitmap.size / 8 + 1;
    read_partition(p, p->sb.inode_bitmap.bits, p->sb.inode_bitmap_lba, size);
}

void load_super_block(partition *p)
{
    super_block *super_b = &p->sb;
    disk_dev[p->my_disk->dev_index].type->read(&(disk_dev[p->my_disk->dev_index]), p->start_lba, super_b, sizeof(super_block));
    if (super_b->magic == SUPERBLOCKMAGIC)
    {
        malloc_mem(p);
        initInodeBuff(p); // 初始化buff
        load_block_bitmap(p);
        load_inode_bitmap(p);
    }
}

// 保存超级块,以及对应的bitmap
void save_super_block(partition *p)
{
    save_block_bitmap(p);
    save_inode_bitmap(p);
    write_partition(p, &p->sb, p->sb.part_lba_base, sizeof(super_block));
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
    super_b->block_bitmap.size = p->sec_cnt;
    // bitmap的大小
    // 计算所用字节
    super_b->block_bitmap_sects = p->sb.block_bitmap.size % 8 == 0 ? p->sb.block_bitmap.size / 8 : p->sb.block_bitmap.size / 8 + 1;
    // 计算扇区数量
    super_b->block_bitmap_sects = super_b->block_bitmap_sects % 512 == 0 ? super_b->block_bitmap_sects / 512 : super_b->block_bitmap_sects / 512 + 1;
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
    super_b->inode_global_start_index = 1; // 不可能只有一个inode的分区,所以以1判断一个分区是否已经挂载过

    malloc_mem(p);
    // 初始化inodebuff
    initInodeBuff(p);
    // 清理
    clearBitmap(&super_b->inode_bitmap);
    clearBitmap(&super_b->block_bitmap);
    // 设置首个inode节点
    setBit(&super_b->inode_bitmap, 0);
    // 设置已使用扇区
    for (uint32_t i = 0; i < super_b->data_start_lba - p->start_lba; i++)
    {
        setBit(&super_b->block_bitmap, i);
    }

    save_super_block(p);
}