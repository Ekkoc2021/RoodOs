
#include "../include/str.h"
#define false 0
#define true 1
#define bool char
#define NULL 0x0

// 实现 shell 命令行
// 测试设备管理系统调用
enum f_types
{
    FT_UNKNOWN,   // 不支持的文件类型
    FT_REGULAR,   // 普通文件
    FT_DIRECTORY, // 目录
    DEVICE,       // 设备
    PARTITION,    // 挂载的分区
};
typedef struct
{
    char *buf;
    unsigned int typeId;
    unsigned int deviceId;
    unsigned int addr;
    unsigned int size; // 即是返回值有是传入值
} devParam_;
typedef struct
{
    char *cmd;
    void (*resolve)(char *cmd, char *args);
} command;

devParam_ dev_stdio;

typedef struct
{
    unsigned int fd;
    unsigned int addr;
    char *buf;
    unsigned int size;
} f_param;

typedef struct dir_entry
{
    char filename[16];   // 普通文件或目录名称
    int i_no;            // 普通文件或目录对应的inode编号
    enum f_types f_type; // 文件类型
} dir_en;

typedef struct
{
    unsigned short weight;
    unsigned short argsLength;
    char *exec_file_path;
    char *args;
} proc_inf;

void print(char *mess)
{
    asm volatile(
        "movl $1, %%eax\n"
        "movl %0, %%ebx\n"
        "int $0x30\n"
        :
        : "r"(mess)
        : "%eax", "%ebx");
}

void open_stdio()
{
    dev_stdio.typeId = 1;
    dev_stdio.deviceId = 0;
    dev_stdio.size = 128;
    asm volatile(
        "movl $60, %%eax\n"
        "movl %0, %%ebx\n"
        "int $0x30\n"
        :
        : "r"(&dev_stdio)
        : "%eax", "%ebx");
}
unsigned int read_stdio(char *buff, unsigned int size)
{
    dev_stdio.size = size;
    dev_stdio.buf = buff;
    unsigned int read_size = 0;
    asm volatile(
        "movl $61, %%eax\n"
        "movl %0, %%ebx\n"
        "movl %1, %%ecx\n"
        "int $0x30\n"

        :
        : "r"(&dev_stdio), "r"(&read_size)
        : "%eax", "%ebx", "%ecx");
    return read_size;
}

unsigned int write_stdio(char *buff, unsigned int size)
{
    dev_stdio.size = size;
    dev_stdio.buf = buff;
    unsigned int write_size = 0;
    asm volatile(
        "movl $62, %%eax\n"
        "movl %0, %%ebx\n"
        "movl %1, %%ecx\n"
        "int $0x30\n"
        :
        : "r"(&dev_stdio), "r"(&write_size)
        : "%eax", "%ebx", "%ecx");
    return write_size;
}

char output_buff[512];
// ls 列出当前文件夹下的文件
// cd 文件夹
// mkdir 创建文件夹
// mkfile 创建文件
// rm 删除文件
// rmdir 删除文件夹,如果文件夹内有文件,则无法删除
// exec 加载某个可执行的文件

int work_dir_fd;    // 当前位置的文件描述符
char work_dir[256]; // 当前所在位置
unsigned int open_f(char *file_path)
{
    unsigned int return_value;
    asm volatile(
        "movl $100, %%eax\n"
        "movl %0, %%ebx\n"
        "movl %1, %%ecx\n"
        "int $0x30\n"
        :
        : "r"(file_path), "r"(&return_value)
        : "%eax", "%ebx", "%ecx");
    return return_value;
}
void close_f(int fd)
{
    asm volatile(
        "movl $103, %%eax\n"
        "movl %0, %%ebx\n"
        "int $0x30\n"
        :
        : "r"(fd)
        : "%eax", "%ebx");
}
// sector 扇区
int read_f(int fd, char *buff, int sector, int size)
{
    f_param fp;
    fp.fd = fd;
    fp.addr = sector;
    fp.size = size;
    fp.buf = buff;

    int read_size;
    asm volatile(
        "movl $101, %%eax\n"
        "movl %0, %%ebx\n"
        "movl %1, %%ecx\n"
        "int $0x30\n"
        :
        : "r"(&fp), "r"(&read_size)
        : "%eax", "%ebx", "%ecx");
    return read_size;
}

// write
int write_f(int fd, char *buff, int sector, int size)
{
    f_param fp;
    fp.fd = fd;
    fp.addr = sector;
    fp.size = size;
    fp.buf = buff;
    int read_size;

    asm volatile(
        "movl $102, %%eax\n"
        "movl %0, %%ebx\n"
        "movl %1, %%ecx\n"
        "int $0x30\n"
        :
        : "r"(&fp), "r"(&read_size)
        : "%eax", "%ebx", "%ecx");
    return read_size;
}

bool make_d(int fd, char *dir_name)
{
    f_param fp;
    fp.fd = fd;
    fp.buf = dir_name;

    char return_value;
    asm volatile(
        "movl $104, %%eax\n"
        "movl %0, %%ebx\n"
        "movl %1, %%ecx\n"
        "int $0x30\n"
        :
        : "r"(&fp), "r"(&return_value)
        : "%eax", "%ebx", "%ecx");
    return return_value;
}

bool make_f(int fd, char *dir_name)
{
    f_param fp;
    fp.fd = fd;
    fp.buf = dir_name;

    char return_value;
    asm volatile(
        "movl $105, %%eax\n"
        "movl %0, %%ebx\n"
        "movl %1, %%ecx\n"
        "int $0x30\n"
        :
        : "r"(&fp), "r"(&return_value)
        : "%eax", "%ebx", "%ecx");
    return return_value;
}

bool remove_f(int fd, char *file_name)
{
    f_param fp;
    fp.fd = fd;
    fp.buf = file_name;

    char return_value;
    asm volatile(
        "movl $106, %%eax\n"
        "movl %0, %%ebx\n"
        "movl %1, %%ecx\n"
        "int $0x30\n"
        :
        : "r"(&fp), "r"(&return_value)
        : "%eax", "%ebx", "%ecx");
    return return_value;
}

unsigned int get_ft(int fd)
{
    f_param fp;
    fp.fd = fd;
    unsigned int return_value;
    asm volatile(
        "movl $108, %%eax\n"
        "movl %0, %%ebx\n"
        "movl %1, %%ecx\n"
        "int $0x30\n"
        :
        : "r"(&fp), "r"(&return_value)
        : "%eax", "%ebx", "%ecx");
    return return_value;
}
unsigned short exec_(unsigned short weight, char *exec_file_path, unsigned short argsLength)
{
    proc_inf p;
    p.weight = weight;
    p.exec_file_path = exec_file_path;
    p.argsLength = argsLength;
    unsigned short PID;
    asm volatile(
        "movl $13, %%eax\n"
        "movl %0, %%ebx\n"
        "movl %1, %%ecx\n"
        "int $0x30\n"
        :
        : "r"(&p), "r"(&PID)
        : "%eax", "%ebx", "%ecx");
    return PID;
}
bool remove_d(int fd, char *dir_name)
{
    f_param fp;
    fp.fd = fd;
    fp.buf = dir_name;

    char return_value;
    asm volatile(
        "movl $107, %%eax\n"
        "movl %0, %%ebx\n"
        "movl %1, %%ecx\n"
        "int $0x30\n"
        :
        : "r"(&fp), "r"(&return_value)
        : "%eax", "%ebx", "%ecx");
    return return_value;
}
char file_buff[512];
char reslove_ls(char *cmd, char *args)
{
    // 在本地文件夹
    int sec_index = 0;
    dir_en *entries = file_buff;
    int file_count = 0;
    dir_en output_dir_en_buff[2]; // 输出缓冲区
    int dir_en_buff_length = 0;
    while (read_f(work_dir_fd, file_buff, sec_index, 512) != 0)
    {

        for (int i = 0; i < 512 / 24; i++)
        {
            if (entries[i].f_type != FT_UNKNOWN)
            {
                memcpy_(output_dir_en_buff + dir_en_buff_length, entries + i, sizeof(dir_en));
                file_count++;
                dir_en_buff_length++;
                if (dir_en_buff_length == 2)
                {
                    char *t1;
                    if (output_dir_en_buff[0].f_type == FT_REGULAR)
                    {
                        t1 = "file";
                    }
                    else
                    {
                        t1 = "dir";
                    }
                    char *t2;
                    if (output_dir_en_buff[1].f_type == FT_REGULAR)
                    {
                        t2 = "file";
                    }
                    else
                    {
                        t2 = "dir";
                    }
                    // 清空buff 64字节的数据
                    for (int j = 0; j < 64; j++)
                    {
                        output_buff[j] = ' ';
                    }
                    output_buff[64] = '\0';
                    output_buff[63] = '\n';
                    char *temp_buff = output_buff + 66;
                    sprintf_(temp_buff, "  %s [%s]", output_dir_en_buff[0].filename, t1);
                    memcpy_(output_buff, temp_buff, strlen_(temp_buff));
                    sprintf_(temp_buff, "  %s [%s]", output_dir_en_buff[1].filename, t2);
                    memcpy_(output_buff + 30, temp_buff, strlen_(temp_buff));
                    write_stdio(output_buff, strlen_(output_buff));
                    dir_en_buff_length = 0;
                }
            }
        }
        sec_index++;
    }
    if (dir_en_buff_length != 0)
    {
        char *t1;
        if (output_dir_en_buff[0].f_type == FT_REGULAR)
        {
            t1 = "file";
        }
        else
        {
            t1 = "dir";
        }
        sprintf_(output_buff, "  %s [%s]", output_dir_en_buff[0].filename, t1);
        write_stdio(output_buff, strlen_(output_buff));
    }

    sprintf_(output_buff, "\ntotal   %d  \n", file_count);
    write_stdio(output_buff, strlen_(output_buff));
    return 1;
}
char reslove_cd(char *cmd, char *args)
{

    // 处理args
    char *file_path = args;
    // 解析参数
    for (int index = 0; index < 256; index++)
    {
        if (args[index] == ' ')
        {
            file_path = args + index + 1;
        }
        else
        {
            break;
        }
    }
    if (file_path[0] == '\0')
    {
        return 0;
    }
    int work_dir_length = strlen_(work_dir);
    for (int i = 0; i < 256; i++)
    {
        if (file_path[i] == ' ' || file_path[i] == '\0')
        {
            file_path[i] = '\0';
            if (file_path[i - 1] == '/')
            {
                // 处理 /hello/ 的情况
                if (i != 1)
                {
                    file_path[i - 1] = '\0';
                }
            }
            break;
        }
    }
    if (strcmp_(".", file_path) == 0)
    {
        return true;
    }
    if (strcmp_("..", file_path) == 0)
    {

        if (work_dir[work_dir_length - 1] == '/')
        {
            return true;
        }

        // 找到最后一个 / 重置为 \0
        for (int i = work_dir_length - 1; i >= 0; i--)
        {
            if (work_dir[i] == '/')
            {
                close_f(work_dir_fd); // 关闭原来的fd
                // file_buff得到新的工作目录
                if (i == 0)
                {
                    work_dir[1] = '\0';
                }
                else
                {
                    work_dir[i] = '\0';
                }
                work_dir_fd = open_f(work_dir);
                return true;
            }
        }
    }

    // 判断是否基于当前目录
    if (file_path[0] != '/')
    {
        // 基于当前目录
        if (work_dir_length == 1)
        {
            sprintf_(file_buff, "/%s", file_path);
        }
        else
        {
            sprintf_(file_buff, "%s/%s", work_dir, file_path);
        }
    }
    else
    {
        sprintf_(file_buff, file_path);
        if (strcmp_(file_buff, work_dir) == 0)
        {
            return true;
        }
    }

    // file_buff得到新的工作目录
    int new_fd = open_f(file_buff);

    if (new_fd == 1024)
    {
        // 检查文件描述符是否有效
        sprintf_(output_buff, "cd: No such directory (%s)\n", file_buff);
        write_stdio(output_buff, strlen_(output_buff));
        return false;
    }
    // 检查文件类型是否是文件夹
    unsigned int ft = get_ft(new_fd);
    if (ft != FT_DIRECTORY)
    {
        // 检查文件描述符是否有效
        sprintf_(output_buff, "cd: No such directory (%s)\n", file_buff);
        write_stdio(output_buff, strlen_(output_buff));
        close_f(new_fd);
        return false;
    }

    // 满足切换条件
    close_f(work_dir_fd); // 关闭原来的fd
    work_dir_fd = new_fd;
    strcpy_(work_dir, file_buff);
    return true;
}
char reslove_mkdir(char *cmd, char *args)
{
    char *file_name = args;
    // 解析参数
    for (int index = 0; index < 256; index++)
    {
        if (args[index] == ' ')
        {
            file_name = args + index + 1;
            continue;
        }
        else
        {
            break;
        }
    }
    if (file_name[0] == '\0')
    {
        return 0;
    }

    for (int i = 0; i < 256; i++)
    {
        if (file_name[i] == ' ' || file_name[i] == '\0')
        {
            file_name[i] = '\0';
            break;
        }
    }

    return make_d(work_dir_fd, file_name);
}
char reslove_mkfile(char *cmd, char *args)
{
    char *file_name = args;
    // 解析参数
    for (int index = 0; index < 256; index++)
    {
        if (args[index] == ' ')
        {
            file_name = args + index + 1;
            continue;
        }
        else
        {
            break;
        }
    }
    if (file_name[0] == '\0')
    {
        return 0;
    }

    for (int i = 0; i < 256; i++)
    {
        if (file_name[i] == ' ' || file_name[i] == '\0')
        {
            file_name[i] = '\0';
            break;
        }
    }

    return make_f(work_dir_fd, file_name);
}
char reslove_rm(char *cmd, char *args)
{
    char *file_name = args;
    // 解析参数
    for (int index = 0; index < 256; index++)
    {
        if (args[index] == ' ')
        {
            file_name = args + index + 1;
            continue;
        }
        else
        {
            break;
        }
    }
    if (file_name[0] == '\0')
    {
        return 0;
    }

    for (int i = 0; i < 256; i++)
    {
        if (file_name[i] == ' ' || file_name[i] == '\0')
        {
            file_name[i] = '\0';
            break;
        }
    }

    if (!remove_f(work_dir_fd, file_name))
    {
        sprintf_(output_buff, "%s is not a file type \n", file_name);
        write_stdio(output_buff, strlen_(output_buff));
    }
}
char reslove_rmdir(char *cmd, char *args)
{
    char *dir_name = args;
    // 解析参数
    for (int index = 0; index < 256; index++)
    {
        if (args[index] == ' ')
        {
            dir_name = args + index + 1;
            continue;
        }
        else
        {
            break;
        }
    }
    if (dir_name[0] == '\0')
    {
        return 0;
    }

    for (int i = 0; i < 256; i++)
    {
        if (dir_name[i] == ' ' || dir_name[i] == '\0')
        {
            dir_name[i] = '\0';
            break;
        }
    }

    if (!remove_d(work_dir_fd, dir_name))
    {
        sprintf_(output_buff, "%s is not a directory type \n", dir_name);
        write_stdio(output_buff, strlen_(output_buff));
    }
}

char reslove_exec(char *cmd, char *args)
{
    // 处理args
    char *exec_file_name = args;
    // 解析参数
    for (int index = 0; index < 256; index++)
    {
        if (args[index] == ' ')
        {
            exec_file_name = args + index + 1;
        }
        else
        {
            break;
        }
    }
    if (exec_file_name[0] == '\0')
    {
        return 0;
    }

    int work_dir_length = strlen_(work_dir);
    for (int i = 0; i < 256; i++)
    {
        if (exec_file_name[i] == ' ' || exec_file_name[i] == '\0')
        {
            exec_file_name[i] = '\0';
            if (exec_file_name[i - 1] == '/')
            {
                // 处理 /hello/ 的情况
                if (i != 1)
                {
                    exec_file_name[i - 1] = '\0';
                }
            }
            break;
        }
    }
    if (strcmp_(".", exec_file_name) == 0 || strcmp_("..", exec_file_name) == 0)
    {
        // 检查类型失败
        return false;
    }
    // 判断是否基于当前目录
    if (exec_file_name[0] != '/')
    {
        // 基于当前目录
        if (work_dir_length == 1)
        {
            sprintf_(file_buff, "/%s", exec_file_name);
        }
        else
        {
            sprintf_(file_buff, "%s/%s", work_dir, exec_file_name);
        }
    }
    else
    {
        sprintf_(file_buff, exec_file_name);
        if (strcmp_(file_buff, work_dir) == 0)
        {
            return true;
        }
    }

    // file_buff得到新的工作目录
    int new_fd = open_f(file_buff);
    if (new_fd == 1024)
    {
        // 检查文件描述符是否有效
        sprintf_(output_buff, "exec: No such file (%s)\n", file_buff);
        write_stdio(output_buff, strlen_(output_buff));
        return false;
    }
    // 检查文件类型是否是文件夹
    unsigned int ft = get_ft(new_fd);
    if (ft != FT_REGULAR)
    {
        // 检查文件描述符是否有效
        sprintf_(output_buff, "exec: No such file (%s)\n", file_buff);
        write_stdio(output_buff, strlen_(output_buff));
        close_f(new_fd);
        return false;
    }

    // 满足切换条件
    close_f(new_fd);
    exec_(3, file_buff, 0);
    return true;
}

#define CMD_LIST_LENGTH 16
command cmd_list[CMD_LIST_LENGTH] = {
    {.cmd = "ls", .resolve = reslove_ls},
    {.cmd = "cd", .resolve = reslove_cd},
    {.cmd = "mkdir", .resolve = reslove_mkdir},
    {.cmd = "mkfile", .resolve = reslove_mkfile},
    {.cmd = "rm", .resolve = reslove_rm},
    {.cmd = "rmdir", .resolve = reslove_rmdir},
    {.cmd = "exec", .resolve = reslove_exec},
};
// 参数解析
char resolve(char *cmd, char *args)
{
    for (int i = 0; i < CMD_LIST_LENGTH; i++)
    {
        if (cmd_list[i].cmd != NULL && strcmp_(cmd, cmd_list[i].cmd) == 0 && cmd_list[i].resolve != NULL)
        {
            cmd_list[i].resolve(cmd, args);
            return true;
        }
    }
    return false;
}
// 根据传入参数解析命令行并响应,返回是否退出shell程序,1退出
char resolve_and_respond(char *command)
{
    // 首先确定是那一条指令,然后根据指令确定其参数
    char *cmd = command;
    char *args = command;
    // 确定cmd_start
    for (int index = 0; index < 256; index++)
    {
        if (command[index] == ' ')
        {
            cmd = command + index + 1;
            continue;
        }
        else
        {
            break;
        }
    }
    if (cmd[0] == '\0')
    {
        return false;
    }
    // 确定cmd的结尾
    for (int index = 0; index < 256; index++)
    {
        if (cmd[index] == ' ' || cmd[index] == '\0')
        {
            cmd[index] = '\0';
            args = cmd + index + 1; // 如果args恰好在哨兵位置,说明到达输入结尾
            break;
        }
    }
    // 确定调用那个命令去处理
    if (!resolve(cmd, args))
    {
        {
            sprintf_(output_buff, "%s : command not found\n", cmd);
            write_stdio(output_buff, strlen_(output_buff));
        }
    }
}
// shell 实现
// 一个个从键盘读取字符然后往控制台丢数据,同时记录读取到数据,当按下回车的时候,解析读取到的数据

void shell()
{
    char data;
    char cmd_str[256]; // 最多256个字符,不能再多了
    int cmd_length;
    char *prompt = "roodOs@ekko>";

    work_dir[0] = '/';
    work_dir[1] = '\0';
    work_dir_fd = open_f(work_dir);
    open_stdio();

    while (1)
    {
        // 写入标识
        cmd_length = 0;
        sprintf_(output_buff, "%s:%s >", prompt, work_dir);
        write_stdio(output_buff, strlen_(output_buff));

        while (1)
        {
            // 读取到如果是回车则往缓冲区中放数据
            read_stdio(&data, 1);

            if (data == '\r' || cmd_length == 253)
            {
                data = '\n';
                write_stdio(&data, 1); // 写入换行
                cmd_str[cmd_length] = '\0';
                cmd_str[cmd_length + 1] = '\0'; // 哨兵防止溢出
                break;
            }

            if (data == '\b')
            {

                if (cmd_length != 0)
                {
                    cmd_length--;
                    write_stdio(&data, 1);
                }
                // 如果buff_size为0不管
            }
            else
            {
                cmd_str[cmd_length] = data;
                cmd_length++;
                write_stdio(&data, 1);
            }
        }

        // 解析命令
        resolve_and_respond(cmd_str);

        // 不提供退出命令行的方式
    }
}
void *malloc_page(unsigned int page_size)
{
    void *return_value;
    asm volatile(
        "movl $200, %%eax\n"
        "movl %0, %%ebx\n"
        "movl %1, %%ecx\n"
        "int $0x30\n"
        :
        : "r"(page_size), "r"(&return_value)
        : "%eax", "%ebx", "%ecx");
    return return_value;
}

void exit()
{
    asm volatile(
        "movl $12, %%eax\n"
        "int $0x30\n"
        :
        :
        :);
}
char *banner =
    "         ____                                   ___              ________  \n"
    "       /  ___ \\     ____       _____           /  /   ____     / ________/\n"
    "      / /__ / /  /   ___  \\   /  ___ \\    ____/  /  / ____ \\  / /_____\n"
    "     /____  /   / /    / /  / /    / /  /  __   /  / /    \\ | |_____  \\\n"
    "    / /  \\ |   / /____/ /  / /____/ /  / /__/  /  / /____/ / ______/  /\n"
    "   /_/    \\_\\  \\_______/   \\_______/  |_______/   \\_______/ /________/ \n\n"
    "                                                              20240304-V0.0.1\n\n";
main()
{
    // int i = 0;
    // // exit();
    // while (1)
    // {
    //     i++;
    // }
    // int fd = open_f("/test");
    // char *bu = malloc_page(3);
    // readDis(bu, 10, 500);
    // write_f(fd, bu, 0, 3 * 4096);
    print("\n  welcome !\n");
    print(banner);
    shell();

    return 0;
}
