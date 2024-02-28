// 目录文件相关接口
#include "fs.h"
extern bool register_file_type(file_type *type);

inode *open_dir(uint32_t inode_no, uint32_t mode)
{
    // 打开当前inode,计数器加1,然后保存数据
    inode *ino = load_inode_by_inode_no(inode_no);
    // todo 当前那个文件正在写,加锁?先不管!
    ino->i_open_cnts++;
    return ino;
}
char data_buf[1024];
int32_t read_dir(uint32_t inode_no, uint32_t addr, char *buf, uint32_t size)
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

int32_t write_dir(uint32_t inode_no, uint32_t addr, char *buf, uint32_t size)
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

uint32_t control_dir(uint32_t inode_no, uint32_t cmd, int32_t *args, uint32_t n)
{
}

void info_dir(uint32_t inode_no, char buff[DEVINFOSIZE])
{
}

void close_dir(uint32_t inode_no)
{
    // 打开当前inode,计数器加1,然后保存数据
    inode *ino = load_inode_by_inode_no(inode_no);
    ino->i_open_cnts--;
    save_inode(inode_no);
}

file_type dir_type = {
    .type = FT_DIRECTORY,
    .open = open_dir,
    .read = read_dir,
    .write = write_dir,
    .control = control_dir,
    .info = info_dir,
    .close = close_dir,
};

void copy_filename(char *destination, char *source)
{
    strlen_(source) < MAX_FILE_NAME_LEN ? memcpy_(destination, source, strlen_(source) + 1) : memcpy_(destination, source, MAX_FILE_NAME_LEN - 1);
    destination[MAX_FILE_NAME_LEN] = 0;
}
char dir_buff[1024]; // 后512专门设置为一个空白的inode文件的数据

bool make_dir(uint32_t root_ino, char *dir_name)
{
    // 读取文件
    dir_entry entry;
    entry.f_type = FT_DIRECTORY;
    copy_filename(entry.filename, dir_name);
    uint32_t sec_index = 0;
    dir_entry *d_e = dir_buff;
    int32_t target_sec_index = -1;
    int32_t entry_index = -1;
    while (read_dir(root_ino, sec_index, dir_buff, 512) > 0)
    {
        for (uint32_t i = 0; i < 512 / 24; i++)
        {
            if (target_sec_index < 0 && d_e[i].f_type == FT_UNKNOWN)
            {
                // 找到了空位置
                target_sec_index = sec_index;
                entry_index = i;
            }
            else if (strcmp_(d_e[i].filename, entry.filename) == 0)
            {
                return false;
            }
        }
        sec_index++;
    }
    partition *p = get_partition_by_inode_no(root_ino);
    if (p == NULL)
    {
        return false;
    }
    inode *i = malloc_inode(p);

    if (i == NULL)
    {
        return false;
    }
    // 初始化文件夹
    d_e = dir_buff + 512;
    d_e[0].i_no = i->i_no;
    d_e[0].filename[0] = '.';
    d_e[0].filename[1] = '\0';
    d_e[0].f_type = FT_DIRECTORY;

    d_e[1].i_no = root_ino;
    d_e[1].filename[0] = '.';
    d_e[1].filename[1] = '.';
    d_e[1].filename[2] = '\0';
    d_e[1].f_type = FT_DIRECTORY;
    write_dir(i->i_no, 0, d_e, 512);
    d_e[0].f_type = FT_UNKNOWN;
    d_e[1].f_type = FT_UNKNOWN;

    d_e = dir_buff;
    if (target_sec_index != -1)
    {
        // 找到空位,且没有重名
        read_dir(root_ino, target_sec_index, dir_buff, 512);
        entry.i_no = i->i_no - p->sb.inode_global_start_index;
        memcpy_(d_e + entry_index, &entry, sizeof(dir_entry));
        write_dir(root_ino, target_sec_index, d_e, 512);
        return true;
    }

    // 没有空的且没有空间了
    d_e = dir_buff + 512;
    entry.i_no = i->i_no - p->sb.inode_global_start_index;
    memcpy_(d_e, &entry, sizeof(dir_entry));
    if (write_dir(root_ino, sec_index, d_e, 512) == 0)
    {
        d_e[0].f_type = FT_UNKNOWN;
        free_all_resource_inode(i->i_no);
        return false;
    }
    d_e[0].f_type = FT_UNKNOWN;
    return true;
}

bool make_file(uint32_t root_ino, char *file_name)
{
    // 读取文件
    dir_entry entry;
    entry.f_type = FT_REGULAR;
    copy_filename(entry.filename, file_name);
    uint32_t sec_index = 0;
    dir_entry *d_e = dir_buff;
    int32_t target_sec_index = -1;
    int32_t entry_index = -1;
    while (read_dir(root_ino, sec_index, dir_buff, 512) > 0)
    {
        for (uint32_t i = 0; i < 512 / 24; i++)
        {
            if (target_sec_index < 0 && d_e[i].f_type == FT_UNKNOWN)
            {
                // 找到了空位置
                target_sec_index = sec_index;
                entry_index = i;
            }
            else if (strcmp_(d_e[i].filename, entry.filename) == 0)
            {
                return false;
            }
        }
        sec_index++;
    }
    partition *p = get_partition_by_inode_no(root_ino);
    if (p == NULL)
    {
        return false;
    }
    inode *i = malloc_inode(p);

    if (i == NULL)
    {
        return false;
    }
    // // 初始化文件夹
    // d_e = dir_buff + 512;
    // d_e[0].i_no = i->i_no;
    // d_e[0].filename[0] = '.';
    // d_e[0].filename[1] = '\0';
    // d_e[0].f_type = FT_DIRECTORY;

    // d_e[1].i_no = root_ino;
    // d_e[1].filename[0] = '.';
    // d_e[1].filename[1] = '.';
    // d_e[1].filename[2] = '\0';
    // d_e[1].f_type = FT_DIRECTORY;
    // write_dir(i->i_no, 0, d_e, 512);
    // d_e[0].f_type = FT_UNKNOWN;
    // d_e[1].f_type = FT_UNKNOWN;

    // d_e = dir_buff;
    if (target_sec_index != -1)
    {
        // 找到空位,且没有重名
        read_dir(root_ino, target_sec_index, dir_buff, 512);
        entry.i_no = i->i_no - p->sb.inode_global_start_index;
        memcpy_(d_e + entry_index, &entry, sizeof(dir_entry));
        write_dir(root_ino, target_sec_index, d_e, 512);
        return true;
    }

    // 没有空的且没有空间了
    d_e = dir_buff + 512;
    entry.i_no = i->i_no - p->sb.inode_global_start_index;
    memcpy_(d_e, &entry, sizeof(dir_entry));
    if (write_dir(root_ino, sec_index, d_e, 512) == 0)
    {
        d_e[0].f_type = FT_UNKNOWN;
        free_all_resource_inode(i->i_no);
        return false;
    }
    d_e[0].f_type = FT_UNKNOWN;
    return true;
}

// 删除文件
bool delete_file(uint32_t root_ino, char *file_name)
{
}

char test_buff[1024];
void init_direcory()
{
    dir_entry *d_e = dir_buff + 512;
    // 初始化 dir_buf的空白区域
    for (uint32_t i = 0; i < 512 / 24; i++)
    {
        d_e[i].f_type = FT_UNKNOWN;
    }

    register_file_type(&dir_type);
    d_e = test_buff;
    log(" \n %d \n", sizeof(dir_entry));
    d_e[0].i_no = 0;
    d_e[0].filename[0] = '.';
    d_e[0].filename[1] = '\0';
    d_e[0].f_type = FT_DIRECTORY;

    d_e[1].i_no = 0;
    d_e[1].filename[0] = '.';
    d_e[1].filename[1] = '.';
    d_e[1].filename[2] = '\0';
    d_e[1].f_type = FT_DIRECTORY;
    write_dir(0, 0, test_buff, 512);
    make_dir(0, "test_file");
    read_dir(0, 0, test_buff, 512);

    for (uint32_t i = 0; i < 512 / 24; i++)
    {
        if (d_e[i].f_type != FT_UNKNOWN)
        {
            log("type:%d name:%s i_no:%d \n", d_e[i].f_type, d_e[i].filename, d_e[i].i_no);
        }
    }
}