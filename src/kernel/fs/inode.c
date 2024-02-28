#include "fs.h"
extern device disk_dev[DISKMAXLENGTH];
extern uint32_t diskCount;                      // 磁盘数量
extern partition *all_partition[PARTITIONSIZE]; // 所有分区表
extern uint32_t partition_cnt;                  // 分区数量

extern file_sys fs; // 文件系统对象

// 通过inode编号找到partition
partition *get_partition_by_inode_no(uint32_t inode_no)
{
    partition *temp = NULL;

    for (uint32_t i = 0; i < fs.amount_partions_length; i++)
    {
        temp = fs.amount_partions[i];
        if (inode_no >= temp->sb.inode_global_start_index &&
            inode_no < (temp->sb.inode_cnt - temp->sb.inode_global_start_index))
        {
            return temp;
        }
    }
    return NULL;
}
// 在缓存中搜索inode
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

// 保存缓存中连续的7个inode,inode编号为相对编号,不是相对于文件系统的绝对编号
void save_7_inode(partition *p, inode *ino)
{
    // 计算相对编号
    uint32_t i_no = ino->i_no - p->sb.inode_global_start_index;
    // 根据ino编号推导扇区号
    uint32_t sec = p->sb.inode_table_lba + i_no / 7;

    // 先检查当前inode位置
    uint32_t offset = i_no % 7;
    ino = ino - offset;
    // 重置编号
    for (uint32_t i = 0; i < 7; i++)
    {
        ino[i].i_no = ino[i].i_no - p->sb.inode_global_start_index;
    }

    // 写入数据
    write_partition(p, ino, sec, sizeof(inode) * 7);

    // 编号改回去
    for (uint32_t i = 0; i < 7; i++)
    {
        ino[i].i_no = ino[i].i_no + p->sb.inode_global_start_index;
    }
}

// 读取inode到内存
// 一次读取7个inode,然后给出的inode编号
inode *load_inode_by_inode_no(uint32_t inode_no)
{
    partition *p = get_partition_by_inode_no(inode_no);
    if (p == NULL)
    {
        return NULL;
    }
    if (!testBit(&p->sb.inode_bitmap, inode_no - p->sb.inode_global_start_index))
    {
        // 说明对应的inode无效
        return NULL;
    }

    // 先查缓存
    inode *target = searchInodeInBuff(p, inode_no);
    if (target != NULL)
    {
        // 缓存命中
        return target;
    }

    // 缓存未找到
    // 计算inode偏移扇区
    uint32_t sec = (inode_no - p->sb.inode_global_start_index) / 7 + p->sb.inode_table_lba;
    // 扇区内的偏移位置
    inode_no = (inode_no - p->sb.inode_global_start_index) % 7;
    // 缓冲区
    char buff[512];
    read_partition(p, buff, sec, 512);
    inode *temp = buff;

    // 循环队列满了,开头7个出队并保存
    if (cirIsFull(&p->openInode))
    {
        inode *front;
        cirFront(&p->openInode, &front);
        save_7_inode(p, front);
        for (uint32_t i = 0; i < 7; i++)
        {
            cirDequeue(&p->openInode, &front);
            // 设置没有进行过设置inode的编号
            front->i_no = p->sb.inode_cnt + p->sb.inode_global_start_index + 1;
        }
    }

    // 将读取到的inode放入缓存
    for (uint16_t i = 0; i < 7; i++)
    {
        // 修改inode编号
        // 可用则修改编号,不可用则修改为inode的上界
        if (testBit(&(p->sb.inode_bitmap), (sec - p->sb.inode_table_lba) * 7 + i))
        {
            temp[i].i_no = (sec - p->sb.inode_table_lba) * 7 + i + p->sb.inode_global_start_index;
        }
        else
        {
            temp[i].i_no = p->sb.inode_cnt + 1;
        }

        cirEnqueue(&p->openInode, temp + i);
        if (i == inode_no && testBit(&p->sb.inode_bitmap, temp[i].i_no - p->sb.inode_global_start_index))
        {
            cirRear(&p->openInode, &target);
        }
    }

    return target;
}

// 保存inode
void save_inode(uint32_t inode_no)
{
    // 搜索inode是否在缓冲区内
    partition *p = get_partition_by_inode_no(inode_no);
    if (p == NULL)
    {
        return;
    }
    // 查缓存,缓存有再保存
    inode *target = searchInodeInBuff(p, inode_no);
    if (target != NULL)
    {
        // 队列每次都只保存7个
        save_7_inode(p, target);
    }
}

// 给一个inode分配一个的sector
bool malloc_block(partition *p, inode *node, uint32_t sec_index)
{
    // 如果是前12个
    char temp_buf[512];
    if (sec_index < 11)
    {
        if (node->i_sectors[sec_index] == 0)
        {
            // 找到可设置位置
            int32_t ffb = find_fist_bit(&(p->sb.block_bitmap));
            if (ffb == -1)
            {
                return false;
            }

            setBit(&(p->sb.block_bitmap), ffb);
            save_block_bitmap(p);
            save_super_block(p);
            node->i_sectors[sec_index] = ffb + p->start_lba;
            // 保存inode
            save_inode(node->i_no);
        }

        return true;
    }
    else if (sec_index < (11 + 128))
    {
        // 二级
        if (node->i_sectors[11] == 0)
        {
            // 找到可设置位置
            int32_t ffb = find_fist_bit(&(p->sb.block_bitmap));
            if (ffb == -1)
            {
                return false;
            }
            setBit(&(p->sb.block_bitmap), ffb);

            // save_block_bitmap(p);
            // save_super_block(p);

            node->i_sectors[11] = ffb + p->start_lba;
            for (uint16_t i = 0; i < 512; i++)
            {
                temp_buf[i] = 0;
            }
            // 清空
            write_partition(p, temp_buf, node->i_sectors[11], 512);
            // 保存inode
            save_inode(node->i_no);
        }

        read_partition(p, temp_buf, node->i_sectors[11], 512);
        if (((uint32_t *)temp_buf)[sec_index - 11] == 0)
        {
            // 找到可设置位置
            int32_t ffb = find_fist_bit(&(p->sb.block_bitmap));
            if (ffb == -1)
            {
                return false;
            }
            setBit(&(p->sb.block_bitmap), ffb);
            save_block_bitmap(p);
            save_super_block(p);
            ((uint32_t *)temp_buf)[sec_index - 11] = ffb + p->start_lba;
            // 写回
            write_partition(p, temp_buf, node->i_sectors[11], 512);
        }
        return true;
    }
}

// 回收inode的一个sector
// 没有修改node的大小
void free_block(partition *p, inode *node, uint32_t sec_index)
{
    // 如果是前12个
    char temp_buf[512];
    if (sec_index < 11)
    {
        if (node->i_sectors[sec_index] != 0)
        {
            // 找到可设置位置
            clearBit(&(p->sb.block_bitmap), node->i_sectors[sec_index] - p->start_lba);
            node->i_sectors[sec_index] = 0;
            save_block_bitmap(p);
            save_super_block(p);
            // 保存inode
            save_inode(node->i_no);
        }
    }
    else if (sec_index < (11 + 128))
    {
        // 二级
        if (node->i_sectors[11] == 0)
        {
            return;
        }
        read_partition(p, temp_buf, node->i_sectors[11], 512);
        if (((uint32_t *)temp_buf)[sec_index - 11] != 0)
        {
            // 找到可设置位置
            clearBit(&(p->sb.block_bitmap), ((uint32_t *)temp_buf)[sec_index - 11] - p->start_lba);
            save_block_bitmap(p);
            save_super_block(p);
            ((uint32_t *)temp_buf)[sec_index - 11] = 0;
            // 写回
            write_partition(p, temp_buf, node->i_sectors[11], 512);
        }
    }
}

// 往inode对应的sector中写入数据
void write_inode(uint32_t inode_no, char *buff, uint32_t sec_index)
{
    // 将编号转为分区
    partition *p = get_partition_by_inode_no(inode_no);
    inode *node = load_inode_by_inode_no(inode_no);
    if (node == NULL)
    {
        return;
    }

    // 判断读取位置
    // 如果是前12个
    char temp_buf[512];
    if (sec_index < 11)
    {

        if (node->i_sectors[sec_index] == 0)
        {
            // 分配一个扇区
            if (!malloc_block(p, node, sec_index))
            {
                // 磁盘容量不够了!
                return false;
            }
        }
        write_partition(p, buff, node->i_sectors[sec_index], 512);
        return true;
    }
    else if (sec_index < (11 + 128))
    {
        // 二级
        if (node->i_sectors[11] == 0)
        {
            // 分配一个扇区
            if (!malloc_block(p, node, 11))
            {
                // 磁盘容量不够了!
                return false;
            }
        }

        read_partition(p, temp_buf, node->i_sectors[11], 512);
        if (((uint32_t *)temp_buf)[sec_index - 11] == 0)
        {
            // 分配一个扇区
            if (!malloc_block(p, node, sec_index))
            {
                // 磁盘容量不够了!
                return false;
            }
            read_partition(p, temp_buf, node->i_sectors[11], 512);
        }
        write_partition(p, buff, ((uint32_t *)temp_buf)[sec_index - 11], 512);
        return true;
    }

    return false;
}

// 往inode中sector读取数据
void read_inode(uint32_t inode_no, char *buff, uint32_t sec_index)
{
    // 将编号转为分区
    partition *p = get_partition_by_inode_no(inode_no);
    inode *node = load_inode_by_inode_no(inode_no);
    if (node == NULL)
    {
        return;
    }

    // 判断读取位置
    // 如果是前12个
    char temp_buf[512];
    if (sec_index < 11)
    {

        if (node->i_sectors[sec_index] == 0)
        {
            // 分配一个扇区
            if (!malloc_block(p, node, sec_index))
            {
                // 磁盘容量不够了!
                return false;
            }
        }
        read_partition(p, buff, node->i_sectors[sec_index], 512);
        return true;
    }
    else if (sec_index < (11 + 128))
    {
        // 二级
        if (node->i_sectors[11] == 0)
        {
            // 分配一个扇区
            if (!malloc_block(p, node, 11))
            {
                // 磁盘容量不够了!
                return false;
            }
        }

        read_partition(p, temp_buf, node->i_sectors[11], 512);
        if (((uint32_t *)temp_buf)[sec_index - 11] == 0)
        {
            // 分配一个扇区
            if (!malloc_block(p, node, sec_index))
            {
                // 磁盘容量不够了!
                return false;
            }
            read_partition(p, temp_buf, node->i_sectors[11], 512);
        }
        read_partition(p, buff, ((uint32_t *)temp_buf)[sec_index - 11], 512);
        return true;
    }

    return false;
}

// 分配inode
inode *malloc_inode(partition *p)
{
    int32_t ffb = find_fist_bit(&(p->sb.inode_bitmap));
    if (ffb == -1)
    {
        // 分配
        return NULL;
    }
    setBit(&(p->sb.inode_bitmap), ffb);
    save_inode_bitmap(p);
    save_super_block(p);
    // 初始化一个inode
    inode *i = load_inode_by_inode_no(ffb + p->sb.inode_global_start_index);
    if (i == NULL)
    {
        return i;
    }

    i->i_open_cnts = 0;
    for (uint16_t j = 0; j < INODE_SECTORS_SIZE; j++)
    {
        i->i_sectors[j] = 0;
    }
    i->i_size = 0;
    i->write_deny = 0;
    return i;
}

// 释放inode
void free_inode(uint32_t inode_no)
{
    partition *p = get_partition_by_inode_no(inode_no);
    inode *node = load_inode_by_inode_no(inode_no);
    if (node == NULL)
    {
        return;
    }

    clearBit(&(p->sb.inode_bitmap), inode_no - p->sb.inode_global_start_index);
    save_inode_bitmap(p);
    save_super_block(p);
    node->i_no = p->sb.inode_global_start_index + p->sb.inode_cnt;
}

// 传入扇区索引(整个磁盘内的位置),分区
// 释放该扇区
bool return_sector(partition *p, uint32_t sec)
{
    if (sec > p->start_lba && sec - p->start_lba < p->sb.sec_cnt)
    {
        clearBit(&(p->sb.block_bitmap), sec - p->start_lba < p->sb.sec_cnt);
        // 持久化资源
        save_block_bitmap(p);
        save_super_block(p);
        return true;
    }
    return false;
}

// 释放一个inode节点得到磁盘资源,包括释放inode本身占用
void free_all_resource_inode(uint32_t inode_no)
{
    partition *p = get_partition_by_inode_no(inode_no);
    inode *node = load_inode_by_inode_no(inode_no);

    for (uint32_t i = 0; i < INODE_SECTORS_SIZE; i++)
    {
        if (node->i_sectors[i] != 0)
        {
            // 释放一级索引
            if (i < 11)
            {
                return_sector(p, node->i_sectors[i]);
            }
            else if (i == 11)
            {
                // 释放二级索引
                uint32_t buff[128];
                read_partition(p, buff, node->i_sectors[11], 512);
                for (uint32_t j = 0; j < 128; j++)
                {
                    // 分配inode是连续的
                    if (buff[j] != 0)
                    {
                        return_sector(p, buff[j]);
                        buff[j] = 0;
                    }
                    else
                    {
                        return_sector(p, node->i_sectors[11]);
                        node->i_sectors[11] = 0;
                        break;
                    }
                }
            }
        }
        else
        {
            // 分配是连续的
            free_inode(node->i_no);
            return;
        }
    }
}
