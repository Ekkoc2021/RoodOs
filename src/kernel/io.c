#define VGALOACTION 0xc00b8000

#include "include/io.h"

#define COM1_PORT 0x3F8

//----------------------这几个中断的函数怎么放这里啊!!!------------ todo 修改
void disable_ir()
{
    asm volatile("cli");
}

uint32_t areInterruptsEnable()
{
    uint32_t flags;
    asm volatile("pushf; pop %0" : "=g"(flags));
    return flags & (1 << 9);
}
// 可能会存在,嵌套函数频繁开关中断,enable_irq应该要考虑原中断状态
void enable_ir()
{
    //
    asm volatile("sti");
}
// 确保中断关闭,如果关闭这返回1,否则关闭中断且返回0
char BeSureDisable_ir()
{
    if (areInterruptsEnable())
    {
        disable_ir();
        return 0;
    }
    return 1;
}
void Resume_ir(char status)
{
    if (status == 0)
    {
        enable_ir();
    }
}
//----------------------这几个中断的函数怎么放这里啊!!!------------ todo 修改

// 配置qemu输出到串口,同时定位到终端,方便查看日志
void log_init(void)
{
    outb(COM1_PORT + 1, 0x00);
    outb(COM1_PORT + 3, 0x80);
    outb(COM1_PORT + 0, 0x3);
    outb(COM1_PORT + 1, 0x00);
    outb(COM1_PORT + 3, 0x03);
    outb(COM1_PORT + 2, 0xc7);
    outb(COM1_PORT + 4, 0x0F);
}

void log_qemu_puts(char *s)
{
    uint32_t i = 0;
    while (0 == 0)
    {
        if (s[i] == '\0' || i > 1024)
        {
            break;
        }
        log_qemu_putchar(s[i]);
        i++;
    }
}

void log_qemu_putchar(char out)
{
    if (out == '\n')
    {
        outb(COM1_PORT, '\r');
    }

    outb(COM1_PORT, out);
}

void log_qemu_printf(const char *format, ...)
{
    // 逐个参数往str中拷贝
    char s = BeSureDisable_ir();
    va_list args;           // 定义参数
    va_start(args, format); // 初始化
    char buff[64];          // 缓冲区 数值转字符串缓冲区

    // char str[512]; // 格式化后输出字符串

    int32_t i = 0;
    while (format[i] != '\0')
    {
        if (format[i] != '%')
        {
            // str[l] = format[i];
            log_qemu_putchar(format[i]);
        }
        else
        {
            char f = format[i + 1];

            switch (f)
            {
            case 'd':
                i++;
                itos_(buff, va_arg(args, sizeT));
                // l = l - 1 + strcpy_(str + l, buff);
                log_qemu_puts(buff);
                // d ==整数
                break;
            case 'f':
                i++;
                // f ==小数
                double tempI = va_arg(args, double);
                ftos_(buff, tempI, 0);
                // l = l - 1 + strcpy_(str + l, buff);
                log_qemu_puts(buff);
                break;
            case 's':
                i++;
                // l = l - 1 + strcpy_(str + l, va_arg(args, char *));
                log_qemu_puts(va_arg(args, char *));
                // s == 字符串
                break;
            case 'c':
                i++;
                // str[l] = (char)va_arg(args, int);
                log_qemu_putchar((char)va_arg(args, int));
                // c == 字符
                break;
            case 'p':
                i++;
                uiToHexStr((sizeT)va_arg(args, sizeT), buff);
                // l = l - 1 + strcpy_(str + l, buff);
                log_qemu_puts(buff);
                // d ==整数
                break;
            default:
                // 说明%后并非有效格式
                // str[l] = format[i];
                log_qemu_putchar(format[i]);
                break;
            }
        }
        i++;
    }
    va_end(args);
    Resume_ir(s);
}

void printf(const char *format, ...)
{
    // 逐个参数往str中拷贝
    char s = BeSureDisable_ir();
    va_list args;           // 定义参数
    va_start(args, format); // 初始化
    char buff[64];          // 缓冲区 数值转字符串缓冲区

    // char str[512]; // 格式化后输出字符串

    int32_t i = 0;
    while (format[i] != '\0')
    {
        if (format[i] != '%')
        {
            // str[l] = format[i];
            putchar(format[i]);
        }
        else
        {
            char f = format[i + 1];

            switch (f)
            {
            case 'd':
                i++;
                itos_(buff, va_arg(args, sizeT));
                // l = l - 1 + strcpy_(str + l, buff);
                print(buff);
                // d ==整数
                break;
            case 'f':
                i++;
                // f ==小数
                double tempI = va_arg(args, double);
                ftos_(buff, tempI, 0);
                // l = l - 1 + strcpy_(str + l, buff);
                print(buff);
                break;
            case 's':
                i++;
                // l = l - 1 + strcpy_(str + l, va_arg(args, char *));
                print(va_arg(args, char *));
                // s == 字符串
                break;
            case 'c':
                i++;
                // str[l] = (char)va_arg(args, int);
                putchar((char)va_arg(args, int));
                // c == 字符
                break;
            case 'p':
                i++;
                uiToHexStr((sizeT)va_arg(args, sizeT), buff);
                // l = l - 1 + strcpy_(str + l, buff);
                print(buff);
                // d ==整数
                break;
            default:
                // 说明%后并非有效格式
                // str[l] = format[i];
                putchar(format[i]);
                break;
            }
        }
        i++;
    }
    va_end(args);
    Resume_ir(s);
}

// 打印int类型
void pInt(int a)
{
    if (a == 0)
    {
        putchar(a + 48);
        return;
    }

    char isPrint = 0;
    int x = 1000000000;
    char p = 0;
    // a依次出x,打印商,a等于余数
    for (int i = 0; i < 10; i++)
    {

        p = a / x; // p为商
        a = a % x; // a等a的余数

        if (isPrint == 0 && p != 0)
        {
            isPrint = 1; // 可以开始打印了
        }
        // p + 48
        if (isPrint)
        {
            p = p + 48;
            putchar(p);
        }

        x = x / 10;
        /* code */
    }
}

// 实现一个putchar函数

void putchar(char p)
{
    char str[2];
    str[0] = p;
    str[1] = 0;
    print(str);
}

// 实现一个清屏函数
void clear()
{
    // __asm__("xchg %%bx,%%bx" ::); // 断点
    int index = getCInd();
    int count = index / 80; // 得到光标在哪一行

    // 滚动count次
    for (int i = 0; i < count; i++)
    {
        scroll();
        index = index - 80;
        setCInd(index);
    }
}

// 实现一个向上滚动一行的函数
void scroll()
{

    char *address = (char *)VGALOACTION;
    short source = 80 * 2; // 第二行
    short destination = 0;

    int count = getCInd() * 2;
    for (int i = 0; i < count + 1; i++)
    {
        address[destination] = address[source];
        source = source + 1;
        destination = destination + 1;
    }
}

// 基础的打印函数:能够换行
void print(char *message)
{
    // 在光标处写入数据
    char *show = (char *)VGALOACTION;
    int i = 0;
    int cursorIndex = getCInd() * 2;
    int column = 0;

    while (0 == 0)
    {
        if (message[i] == 0)
            break;

        if (message[i] == 0x0d)
        {
            // 如果是回车 0x0a:光标位置,回到行首
            cursorIndex = ((cursorIndex / 2) / 80) * 80 * 2;
            setCInd(cursorIndex / 2);
            i++;
            continue;
        }

        if (message[i] == 0x0a)
        {

            // 如果是换行 0x0d 也就是\n,就意味着进行了一次回车换行:光标+80x2
            // index/2 /80 =第几行
            // 回车行首:==>光标位置 行x80

            // 换行应该检测如果检测是否是最后一行:24行
            column = (cursorIndex / 2) / 80;
            if (column == 24)
            {
                scroll(); // 向上滚动一行
                column--;
            }
            cursorIndex = (column + 1) * 80 * 2;
            setCInd(cursorIndex / 2);
            i++;
            continue;
        }

        if (message[i] == '\b')
        {
            cursorIndex = cursorIndex - 2;
            setCInd(cursorIndex / 2);
            show[cursorIndex] = ' ';
            i++;
            continue;
        }

        show[cursorIndex] = message[i];
        cursorIndex = cursorIndex + 2;
        // 检查光标位置
        column = (cursorIndex / 2) / 80;
        if (column == 25)
        {
            scroll(); // 向上滚动一行
            cursorIndex = 24 * 80 * 2;
        }
        setCInd(cursorIndex / 2);
        i++;
    }
}

int getCInd()
{
    int index = 0;
    getCursorIndex(); // 光标位置放在bx中,返回
    asm __volatile__("movl %%ebx,%0"
                     : "=r"(index)
                     :
                     : "%ebx");
    return index;
}

void setCInd(int index)
{
    // bx中存放要设置的光标位置
    asm __volatile__("movl %0,%%ebx" ::"r"(index)
                     : "%ebx");
    setCursor();
}

// 读磁盘的函数
char readDis(void *dest, unsigned int sectionSize, unsigned int startSection)
{
    // 设置es
    // asm __volatile__("movl 0,%%es");
    // 设置加载到的位置
    asm __volatile__("movl %0,%%edi" ::"r"(dest)
                     : "%edi");
    // 设置读取多少扇区
    asm __volatile__("movl %0,%%ecx" ::"r"(sectionSize)
                     : "%ecx");
    // 起始扇区
    asm __volatile__("movl %0,%%ebx" ::"r"(startSection)
                     : "%ebx");
    readDisk();
    return 1;
}