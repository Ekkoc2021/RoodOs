// 文件系统
#include "fs.h"

extern device disk_dev[DISKMAXLENGTH];
extern uint32_t diskCount;                      // 磁盘数量
extern partition *all_partition[PARTITIONSIZE]; // 所有分区表
extern uint32_t partition_cnt;                  // 分区数量

extern uint32_t mallocPage_u(memoryMarket *market, uint32_t *paddr);
extern uint32_t mallocPage_k(memoryMarket *market, uint32_t *paddr);
extern void freePage(memoryMarket *market, uint32_t vAddr);
extern void tidy(memoryMarket *market);
extern uint32_t mallocMultpage_k(memoryMarket *market, uint32_t n);
extern uint32_t mallocMultpage_u(memoryMarket *market, uint32_t n);
extern void freeNPage(memoryMarket *market, uint32_t vaddr, uint32_t n);

extern memoryMarket market;
file_sys fs; // 文件系统对象

bool identify_super_b(partition *p)
{
    super_block *super_b = &p->sb;
    if (!p->isload)
    {
        load_super_block(p);
    }

    if (super_b->magic == SUPERBLOCKMAGIC)
    {
        return true;
    }
    return false;
}

// 使用支持文件描述符的接口通过文件描述符中文件信息读写文件,
// 追加数据,读写等模式的支持也在使用文件描述中给出

bool open_fs(uint32_t ino, enum file_types ft, uint32_t mode)
{
    // 确定打开的模式,然后返回
    for (uint32_t i = 0; i < fs.file_type_length; i++)
    {
        if (fs.ft[i]->type == ft)
        {
            inode *node = fs.ft[i]->open(ino, mode);
            if (node != NULL)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    return false;
}

// size是缓冲区大小,512字节为单位
int32_t read_fs(uint32_t ino, enum file_types ft, uint32_t addr, char *buf, uint32_t size)
{
    for (uint32_t i = 0; i < fs.file_type_length; i++)
    {
        if (fs.ft[i]->type == ft)
        {
            return fs.ft[i]->read(ino, addr, buf, size);
        }
    }
    return 0;
}

int32_t write_fs(uint32_t ino, enum file_types ft, uint32_t addr, char *buf, uint32_t size)
{
    for (uint32_t i = 0; i < fs.file_type_length; i++)
    {
        if (fs.ft[i]->type == ft)
        {
            return fs.ft[i]->write(ino, addr, buf, size);
        }
    }
    return 0;
}

uint32_t control_fs(uint32_t ino, enum file_types ft, uint32_t cmd, int32_t *args, uint32_t n)
{
    for (uint32_t i = 0; i < fs.file_type_length; i++)
    {
        if (fs.ft[i]->type == ft)
        {
            return fs.ft[i]->control(ino, cmd, args, n);
        }
    }
    return 0;
}

void info_fs(uint32_t ino, enum file_types ft, char buff[DEVINFOSIZE])
{
    for (uint32_t i = 0; i < fs.file_type_length; i++)
    {
        if (fs.ft[i]->type == ft)
        {
            fs.ft[i]->info(ino, buff);
            return;
        }
    }
} // 返回设备文件信息

void close_fs(uint32_t ino, enum file_types ft)
{
    for (uint32_t i = 0; i < fs.file_type_length; i++)
    {
        if (fs.ft[i]->type == ft)
        {
            fs.ft[i]->close(ino);
            return;
        }
    }
}

// 往文件系统内注册文件类型
bool register_file_type(file_type *type)
{
    if (fs.file_type_length == MAX_FILE_TYPE)
    {
        return false;
    }

    // 注意超过长度
    fs.ft[fs.file_type_length] = type;
    fs.file_type_length++;
    return true;
}

//-------读取inode通用接口
inode *open_file(uint32_t inode_no, uint32_t mode)
{
    // 打开当前inode,计数器加1,然后保存数据
    inode *ino = load_inode_by_inode_no(inode_no);
    // todo 当前那个文件正在写,加锁?先不管!
    ino->i_open_cnts++;
    return ino;
}
char *data_buf;
int32_t read_file(uint32_t inode_no, uint32_t addr, char *buf, uint32_t size)
{
    int32_t read_size = 0;
    while (size > 0)
    {
        if (size >= 512)
        {
            if (read_inode(inode_no, buf, addr) == 0)
            {

                return read_size;
            }
            size -= 512;
            read_size += 512;
            buf += 512;
            addr++;
        }
        else
        {
            if (read_inode(inode_no, data_buf, addr) == 0)
            {

                return read_size;
            }
            memcpy_(buf, data_buf, size);
            read_size += size;
            return read_size;
        }
    }
    return read_size;
}

int32_t write_file(uint32_t inode_no, uint32_t addr, char *buf, uint32_t size)
{
    int32_t write_size = 0;
    while (size > 0)
    {
        if (size >= 512)
        {
            if (write_inode(inode_no, buf, addr) == 0)
            {

                return write_size;
            }
            size -= 512;
            write_size += 512;
            buf += 512;
            addr++;
        }
        else
        {
            memcpy_(data_buf, buf, size);
            if (write_inode(inode_no, data_buf, addr) == 0)
            {

                return write_size;
            }

            write_size += size;
            return write_size;
        }
    }
    return write_size;
}

uint32_t control_file(uint32_t inode_no, uint32_t cmd, int32_t *args, uint32_t n)
{
}

void info_file(uint32_t inode_no, char buff[DEVINFOSIZE])
{
}

void close_file(uint32_t inode_no)
{
    // 打开当前inode,计数器加1,然后保存数据
    inode *ino = load_inode_by_inode_no(inode_no);
    ino->i_open_cnts--;
    save_inode(inode_no);
}

// inode.c的函数测试
// void function_test()
// {

//     // 测试往0扇区写入数据
//     char buff[512];
//     char buff2[512];
//     partition *p = get_partition_by_inode_no(0);
//     inode *nod = load_inode_by_inode_no(0);
//     inode *ia = malloc_inode(p);
//     for (uint32_t i = 0; i < 18; i++)
//     {

//         for (uint32_t j = 0; j < 512; j++)
//         {
//             buff[j] = i + 1;
//         }
//         if (i == 11)
//         {
//             log("11\n");
//         }

//         write_inode(ia->i_no, buff, i);
//         read_inode(ia->i_no, buff2, i);
//     }

//     // for (uint32_t i = 0; i < 18; i++)
//     // {
//     //     free_block(p, ia, i);
//     // }
//     free_all_resource_inode(ia->i_no);
//     for (uint32_t i = 0; i < 18; i++)
//     {
//         inode *ia2 = malloc_inode(p);
//         free_inode(ia2->i_no);
//     }
//     free_inode(ia->i_no);
// }

// 维护一个
void fs_init()
{
    data_buf = mallocPage_k(&market, &data_buf);
    log("start init fs \n");
    // 解析分区数据,读取分区根目录,查看挂载详情,至少要有分区
    // 检查分区情况
    // 识别第一个分区的超级块是否有效!
    bool firstInit = false;
    if (!identify_super_b(all_partition[0]))
    {
        // 未成功识别到,构建分区
        buildSuperBlock(all_partition[0], all_partition[0]->sec_cnt / 100 * 5);
        all_partition[0]->sb.inode_global_start_index = 0;
        save_super_block(all_partition[0]);
        firstInit = true;
    }

    //---初始化分区挂载情况
    fs.file_type_length = 0;
    fs.amount_partions_length = 0;
    fs.unknown_partions_length = 0;
    // 从解析到的分区中识别出有效分区
    for (uint32_t i = 0; i < partition_cnt; i++)
    {
        if (!identify_super_b(all_partition[i]))
        {
            // 没有成功识别的分区
            fs.unknown_partions[fs.unknown_partions_length] = all_partition[i];
            fs.unknown_partions_length++;
        }
        else
        {
            // 成功识别的分区
            fs.amount_partions[fs.amount_partions_length] = all_partition[i];
            fs.amount_partions_length++;
        }
    }
    // 进一步整理有效分区:整理分区之间的顺序
    uint32_t inode_global_start_index = 0;
    uint32_t p1 = 0;    // 指向要整理的位置
    uint32_t p2;        // 查找到的分区
    partition *temp;    // 记录找到分区
    bool already_found; // 用于检查是否相似起始索引
    while (true)
    {
        p2 = p1;
        if (p1 == fs.amount_partions_length)
        {
            break;
        }
        already_found = false;
        for (uint32_t i = p2; i < fs.amount_partions_length; i++)
        {
            if (fs.amount_partions[i]->sb.inode_global_start_index == inode_global_start_index)
            {
                if (already_found)
                {
                    // 有相似起始索引
                    // 从p2开始将所有分区都放入到未识别的数组中
                    for (uint32_t j = p1; j < fs.amount_partions_length; j++)
                    {
                        fs.unknown_partions[fs.unknown_partions_length] = fs.amount_partions[j];
                        fs.unknown_partions_length++;
                    }
                    fs.amount_partions_length = p1;
                    goto amount_partition_init_done; // 跳出大循环,结束初始化
                }
                already_found = true;
            }
        }
        if (already_found)
        {
            temp = fs.amount_partions[p1];
            fs.amount_partions[p1] = fs.amount_partions[p2];
            fs.amount_partions[p2] = temp;
        }
        else
        {
            // 没有找到
            for (uint32_t j = p1; j < fs.amount_partions_length; j++)
            {
                fs.unknown_partions[fs.unknown_partions_length] = fs.amount_partions[j];
                fs.unknown_partions_length++;
            }
            fs.amount_partions_length = p1;
            break; // 结束循环
        }
        p1++;
    }
amount_partition_init_done:
    init_direcory();
    init_regular();
    // 如果是第一次初始化
    if (firstInit)
    {
        // 初始化第一个分区表的inode
        inode *nod = load_inode_by_inode_no(0);
        nod->i_no = 0;
        nod->i_open_cnts = 0;
        for (uint32_t i = 0; i < INODE_SECTORS_SIZE; i++)
        {
            nod->i_sectors[i] = 0;
        }
        nod->i_size = 0;
        init_new_dir(0, 0);
    }

    // test
    // function_test();

    log("fs init done! \n");
}

extern processManager manager;

// fd 相关函数
uint32_t find_empty_fd()
{
    file_descriptor *fd = manager.now->file_descriptors;
    for (uint32_t i = 0; i < FD_MEM_SIZE / sizeof(file_descriptor); i++)
    {
        if (fd[i].file_type == FT_UNKNOWN)
        {
            return i;
        }
    }
    return FD_MEM_SIZE / sizeof(file_descriptor);
}

// 根据文件名称打开一个文件
uint32_t syscall_fs_open(char *filepath)
{
    // 找到空fd
    uint32_t fd_index = find_empty_fd();
    if (fd_index == FD_MEM_SIZE / sizeof(file_descriptor))
    {
        return 1024;
    }
    file_descriptor *fd = manager.now->file_descriptors + fd_index;
    if (!search_file_by_path(filepath, &fd->ino, &fd->file_type))
    {
        return 1024;
    }

    if (open_fs(fd->ino, fd->file_type, 0))
    {
        return fd_index;
    }
    fd->file_type = FT_UNKNOWN;
    return 1024;
}

// 根据文件描述符读写文件
uint32_t syscall_write_fs(uint32_t fd, uint32_t addr, char *buf, uint32_t size)
{
    file_descriptor *all_fd = manager.now->file_descriptors;
    if (fd < FD_MEM_SIZE / sizeof(file_descriptor) && all_fd[fd].file_type != FT_UNKNOWN)
    {
        // 可写
        return write_fs(all_fd[fd].ino, all_fd[fd].file_type, addr, buf, size);
    }
    return 0;
}

uint32_t syscall_read_fs(uint32_t fd, uint32_t addr, char *buf, uint32_t size)
{
    file_descriptor *all_fd = manager.now->file_descriptors;
    if (fd < FD_MEM_SIZE / sizeof(file_descriptor) && all_fd[fd].file_type != FT_UNKNOWN)
    {
        // 可写
        return read_fs(all_fd[fd].ino, all_fd[fd].file_type, addr, buf, size);
    }
    return 0;
}

void syscall_close_fs(uint32_t fd)
{
    file_descriptor *all_fd = manager.now->file_descriptors;
    if (fd < FD_MEM_SIZE / sizeof(file_descriptor) && all_fd[fd].file_type != FT_UNKNOWN)
    {
        close_fs(all_fd[fd].ino, all_fd[fd].file_type);
        all_fd[fd].file_type = FT_UNKNOWN;
    }
}

bool make_dir(uint32_t fd, char *dir_name)
{
    file_descriptor *all_fd = manager.now->file_descriptors;
    if (fd < FD_MEM_SIZE / sizeof(file_descriptor) && all_fd[fd].file_type == FT_DIRECTORY)
    {
        return create_dir(all_fd[fd].ino, dir_name);
    }
    return false;
}

bool make_file(uint32_t fd, char *file_name)
{
    file_descriptor *all_fd = manager.now->file_descriptors;
    if (fd < FD_MEM_SIZE / sizeof(file_descriptor) && all_fd[fd].file_type == FT_DIRECTORY)
    {
        return create_file(all_fd[fd].ino, file_name);
    }
    return false;
}

bool remove_file(uint32_t fd, char *file_name)
{
    file_descriptor *all_fd = manager.now->file_descriptors;
    if (fd < FD_MEM_SIZE / sizeof(file_descriptor) && all_fd[fd].file_type == FT_DIRECTORY)
    {
        return delete_file(all_fd[fd].ino, file_name);
    }
    return false;
}

bool remove_dir(uint32_t fd, char *file_name)
{
    file_descriptor *all_fd = manager.now->file_descriptors;
    if (fd < FD_MEM_SIZE / sizeof(file_descriptor) && all_fd[fd].file_type == FT_DIRECTORY)
    {
        return delete_dir(all_fd[fd].ino, file_name);
    }
    return false;
}