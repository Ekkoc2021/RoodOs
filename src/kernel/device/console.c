#include "console.h"

// 实现一个putchar函数
void console_putchar(char p)
{
    // 没有重写putchar的必要
    char str[2];
    str[0] = p;
    str[1] = 0;
    console_puts(str);
}

// 基础的打印函数:能够换行
void console_puts(char *message)
{
    // 在光标处写入数据
    char *console = (char *)VGALOACTION;
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
            console[cursorIndex] = ' ';
            i++;
            continue;
        }

        console[cursorIndex] = message[i];
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
