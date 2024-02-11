// 文件系统
#include "fs.h"

extern device disk_dev[DISKMAXLENGTH];
extern uint32_t diskCount; // 磁盘数量

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

    disk_dev[p->my_disk->dev_index].type->write(&(disk_dev[p->my_disk->dev_index]), super_b->part_lba_base, super_b, sizeof(super_block));
}

bool identify_super_b(partition *p)
{
    super_block *super_b = &p->sb;
    disk_dev[p->my_disk->dev_index].type->read(&(disk_dev[p->my_disk->dev_index]), p->start_lba, super_b, sizeof(super_block));
    if (super_b->magic == SUPERBLOCKMAGIC)
    {
        return true;
    }
    return false;
}
void fs_init()
{
    log("fs_init %d", sizeof(dir_entry));
    // 解析分区数据,读取分区根目录,查看挂载详情
    struct disk *d = disk_dev[0].data;
    // 识别第一个分区的超级块
    if (!identify_super_b(&d->prim_parts[0]))
    {
        // 未成功识别到,构建分区
        buildSuperBlock(&d->prim_parts[0], d->prim_parts[0].sec_cnt / 100 * 5);

        // 放在buildSuperBlock中完成
        // 分配bitmap内存

        // 清空bitmap

        // 设置inodebitmap的第一个inode节点为0==>根分区
    }
    else
    {
        // 分配bitmap内存

        // 读取对应bitmap到指定内存
    }

    // 针对两个
}