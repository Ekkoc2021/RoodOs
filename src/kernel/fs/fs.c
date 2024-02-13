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
    uint32_t sec = p->sb.block_bitmap.size % 8 == 0 ? p->sb.block_bitmap.size / 8 : p->sb.block_bitmap.size / 8 + 1;
    disk_dev[p->my_disk->dev_index].type->write(&(disk_dev[p->my_disk->dev_index]), p->sb.block_bitmap_lba, p->sb.block_bitmap.bits, sec);
}

void load_block_bitmap(partition *p)
{
    uint32_t sec = p->sb.block_bitmap.size % 8 == 0 ? p->sb.block_bitmap.size / 8 : p->sb.block_bitmap.size / 8 + 1;
    disk_dev[p->my_disk->dev_index].type->read(&(disk_dev[p->my_disk->dev_index]),
                                               p->sb.block_bitmap_lba, p->sb.block_bitmap.bits, sec);
}

void save_inode_bitmap(partition *p)
{
    uint32_t sec = p->sb.inode_bitmap.size % 8 == 0 ? p->sb.inode_bitmap.size / 8 : p->sb.inode_bitmap.size / 8 + 1;
    disk_dev[p->my_disk->dev_index].type->write(&(disk_dev[p->my_disk->dev_index]),
                                                p->sb.inode_bitmap_lba, p->sb.inode_bitmap.bits, sec);
}

void load_inode_bitmap(partition *p)
{
    uint32_t sec = p->sb.inode_bitmap.size % 8 == 0 ? p->sb.inode_bitmap.size / 8 : p->sb.inode_bitmap.size / 8 + 1;
    disk_dev[p->my_disk->dev_index].type->read(&(disk_dev[p->my_disk->dev_index]),
                                               p->sb.inode_bitmap_lba, p->sb.inode_bitmap.bits, sec);
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

void initInodeBuff(partition *p)
{
    inode *buff = p->openInode.data.data;
    for (uint32_t i = 0; i < OPENINODESIZE; i++)
    {
        buff[i].i_no = p->sb.inode_cnt;
    }
}

// 加载超级块,同时分配bitmap内存以及加载磁盘bitmap到内存
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

// 保存缓存中连续的7个inode

void save_7_inode(partition *p, inode *ino)
{
    // 根据ino编号推导扇区号
    uint32_t sec = p->sb.inode_table_lba + ino->i_no / 7;

    // 先检查当前inode位置
    uint32_t offset = ino->i_no % 7;
    ino = ino - offset;

    write_partition(p, sec, ino, sizeof(ino) * 7);
}

inode *searchInodeInBuff(partition *p, uint32_t inode_no)
{
    inode *buff = p->openInode.data.data;
    for (uint32_t i = 0; i < OPENINODESIZE - 1; i++)
    {
        if (buff[i].i_no == inode_no)
        {
            return buff + i;
        }
    }
    return NULL;
}

// 读取inode
inode *load_inode(partition *p, uint32_t inode_no)
{

    // 测试inode_no有效
    if (inode_no >= p->sb.inode_cnt || !testBit(&p->sb.inode_bitmap, inode_no))
    {
        return NULL;
    }

    inode *target = searchInodeInBuff(p, inode_no);

    if (target != NULL)
    {
        // 缓存命中
        return target;
    }

    // 计算inode偏移扇区
    uint32_t sec = inode_no / 7;
    sec = sec + p->sb.inode_table_lba;
    // 扇区内的偏移位置
    inode_no = inode_no % 7;
    // 缓冲区
    char buff[512];
    read_partition(p, buff, sec, 512);
    inode *temp = buff;

    // 循环队列满了,开头7个出队
    if (cirIsFull(&p->openInode))
    {
        inode *front;
        cirFront(&p->openInode, &front);
        save_7_inode(p, front);
        for (uint32_t i = 0; i < 7; i++)
        {
            cirDequeue(&p->openInode, &front);
        }
    }

    // 缓存读取到的inode
    for (uint16_t i = 0; i < 7; i++)
    {
        // 修改inode编号
        temp[i].i_no = (sec - p->sb.inode_table_lba) * 7 + i;

        cirEnqueue(&p->openInode, temp + i);
        if (i == inode_no)
        {
            cirRear(&p->openInode, &target);
        }
    }
    return target;
}

// 保存inode到分区
void save_inode(partition *p, inode *i)
{
    // 队列每次都只保存7个
    save_7_inode(p, i);
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
    super_b->block_bitmap_sects = p->sb.block_bitmap.size % 8 == 0 ? p->sb.block_bitmap.size / 8 : p->sb.block_bitmap.size / 8 + 1;
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

file_sys fs; // 文件系统对象

void amount_partion(partition *p)
{
    p->tag.data = p;
    add(&fs.amount_partions, &p->tag);
}

void fs_init()
{
    log("fs_init %d", sizeof(dir_entry));

    initLinkedList(&fs.amount_partions);

    // 解析分区数据,读取分区根目录,查看挂载详情,至少要有分区
    // 检查分区情况
    struct disk *d = disk_dev[0].data;

    // 识别第一个分区的超级块
    if (!identify_super_b(&d->prim_parts[0]))
    {
        // 未成功识别到,构建分区
        buildSuperBlock(&d->prim_parts[0], d->prim_parts[0].sec_cnt / 100 * 5);
    }

    // 针对两个,读取第一个inode
    inode *rootInode = load_inode(&d->prim_parts[0], d->prim_parts[0].sb.root_inode_no);

    // 识别其他分区表,检查是否有挂载

    for (uint32_t i = 0; i < diskCount; i++)
    {
    }
}