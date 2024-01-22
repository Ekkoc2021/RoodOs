/*
 * @Author: EKKO 189890049@qq.com
 * @Date: 2024-01-03 15:29:35
 * @LastEditors: EKKO 189890049@qq.com
 * @LastEditTime: 2024-01-13 20:53:26
 * @Description:
 *      字符串相关: 以0结尾
 *          长度计算:strlen
 *          字符串复制:strncpy
 *          字符串拼接:strncat
 *          字符串比较:strcmp
 *          字符查找:strchr  strrchr
 *          字符串匹配:strstr kmp? 大概率用不到不写了
 *          字符串格式化返回:sprintf
 *          内存设置:memset
 */

#include "str.h"

#ifdef STR_DEBUG
#include <stdio.h>
// stdarg的copy版本版本
// 32位可变参数,是使用栈传参,64位估计有时候不是用栈传参,64位下printInts跑不通!
void printInts(int num, ...)
{
    int *addr = &num; //
    int *temp = addr + 1;
    for (int i = 0; i < num; i++)
    {
        printf("%d ", temp[i]);
    }
    printf("\n");
}

void printInts22(int num, ...)
{
    va_list args;        // 定义参数
    va_start(args, num); // 初始化
    char a = va_arg(args, int);
    for (int i = 0; i < num - 1; i++)
    {
        int value = va_arg(args, int); // 取值然后使用
        printf("%d ", value);
    }

    va_end(args);
    printf("\n");
}

int main(int argc, char const *argv[])
{
    char *s = "hell%o%o world";
    char d[44];
    printf("%s\n", strncpy_(d, s, 11));
    printf("%s\n", strncat_(d, s, 11));
    printf("%d\n", strlen_(s));
    printf("%d\n", strlen_(d));
    char a = 100;
    int d1 = 900;
    printf(strncat_(d, "\n%%d%d\n", 11), a, d1);

    // printInts(3, 10, 20, 30);
    // printInts22(5, 'c', 2, 3, 4, 5);

    sprintf_(d, "\nhell%d%c %s 你好啊%f 世界world %s%c啊%f\n", 0, ':', "你好", -9998.789, "世界", '!', 996145.8922);
    printf(d);
    itos_(d, -999888);
    printf(d);
    printf("\n");
    float f1 = -999888.90;
    ftos_(d, f1, 5);
    printf(d);
    printf("\n%f", -996145.8922);

    printf("\n%d", stoi_("7891123"));
    printf("\n%f\n", stof_("-++ -7891.123"));
    printf("%f\n", stof_("-+ -++789331.002123000"));

    printf("%d\n", strcmp_("afcdsdas我", "afcd你"));

    printf("%s:%d\n", "ajhfasl", strchr_("ajhfasl", 's'));
    char buff[32];
    iToHexStr(95456262, buff);
    printf("%s", buff);
    return 0;
}

#endif
void uiToHexStr(sizeT i, char *buff)
{
    char *hex_char = "0123456789ABCDEF";
    char tempBuff[128];
    sizeT l = 0;
    while (i != 0)
    {
        tempBuff[l] = hex_char[i % 16];
        i = i / 16;
        l++;
        if (l > 127)
        {
            break;
        }
    }
    if (l == 0)
    {
        buff[0] = '0';
        buff[1] = '\0';
        return;
    }

    buff[l] = '\0';
    for (sizeT i = 0; i < l; i++)
    {
        buff[i] = tempBuff[l - i - 1];
    }
}

void iToHexStr(sizeT i, char *buff)
{
    char *hex_char = "0123456789ABCDEF";
    char tempBuff[128];
    char flag = 1; // 正数
    if (i < 0)
    {
        flag = -1;
        i = -i;
    }

    sizeT l = 0;
    while (i != 0)
    {
        tempBuff[l] = hex_char[i % 16];
        i = i / 16;
        l++;
        if (l > 127)
        {
            break;
        }
    }

    if (flag < 0)
    {

        buff[0] = '-';
        buff = buff + 1;
    }

    if (l == 0)
    {
        buff[0] = '0';
        buff[1] = '\0';
        return;
    }

    buff[l] = '\0';
    for (sizeT i = 0; i < l; i++)
    {
        buff[i] = tempBuff[l - i - 1];
    }
}
void memcpy_(char *destination, char *source, sizeT size)
{
    for (sizeT i = 0; i < size; i++)
    {
        destination[i] = source[i];
    }
}

sizeT strchr_(const char *template, char a)
{
    sizeT i = 0;
    while (0 == 0)
    {
        // 会不会有死循环?
        if (template[i] == '\0')
        {
            return -1;
        }
        if (template[i] == a)
        {
            return i;
        }
        i++;
    }
}
char strcmp_(const char *a, const char *b)
{
    for (sizeT i = 0;; i++)
    {
        if (a[i] - b[i] != 0)
        {
            return a[i] - b[i];
        }
        if (a[i] == '\0')
        {
            // 能到这里:b[i]=='\0';
            return 0;
        }
    }
    return 0;
}
sizeT strlen_(const char *str)
{
    sizeT l = 0;
    while (str[l] != '\0') // 有没有可能死循环
    {
        l++;
    }
    return l;
}

char *strncpy_(char *dest, const char *src, sizeT n)
{
    sizeT i = 0;
    for (; i < n; i++)
    {
        dest[i] = src[i];
    }
    dest[i] = '\0';
    return dest;
}

sizeT strcpy_(char *dest, const char *src)
{
    sizeT i = 0;
    while (src[i] != '\0')
    {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return i;
}

char *strncat_(char *dest, const char *src, sizeT n)
{
    sizeT l = strlen_(dest);

    for (sizeT i = 0; i < n; i++)
    {
        dest[l] = src[i];
        l++;
    }
    dest[l] = '\0';
    return dest;
}

double stof_(const char *buff)
{
    // 先作为当做整数处理
    char sign = 0;
    // 清理空白前缀,提取符号
    sizeT offset = 0;
    for (;; offset++)
    {
        if (buff[offset] != ' ')
        {
            if (buff[offset] == '-')
            {
                sign = sign ^ 1; // 相同为0 不同为1,取反
                continue;
            }

            if (buff[offset] == '+')
            {
                continue;
            }
            break;
        }
    }

    buff = buff + offset;

    // string 转 integer
    sizeT digits = 0;
    double temp = 1;
    while (buff[digits] != '\0' && buff[digits] >= '0' && buff[digits] <= '0' + 9)
    {
        temp = temp * 10;
        digits++;
    }
    // 个 位有效 temp=10 digits=1
    // 十 位有效 temp=100 digits=2

    double res = 0;

    for (sizeT i = 0; i < digits; i++)
    {
        temp = temp / 10;
        res = res + (buff[i] - '0') * temp;
    }

    if (buff[digits] != '.')
    {
        return sign ? -res : res;
    }

    // 处理小数部分
    buff = buff + digits + 1;
    digits = 0;
    while (buff[digits] != '\0' && buff[digits] >= '0' && buff[digits] <= '0' + 9)
    {
        temp = temp / 10;
        res = res + (buff[digits] - '0') * temp;
        digits++;
    }
    return sign ? -res : res;
}
sizeT stoi_(const char *buff)
{
    char sign = 0;
    // 清理空白前缀,提取符号
    sizeT offset = 0;
    for (;; offset++)
    {
        if (buff[offset] != ' ')
        {
            if (buff[offset] == '-')
            {
                sign = sign ^ 1; // 相同为0 不同为1,取反
                continue;
            }

            if (buff[offset] == '+')
            {
                continue;
            }
            break;
        }
    }

    buff = buff + offset;

    // string 转 integer
    sizeT digits = 0;
    sizeT temp = 1;
    while (buff[digits] != '\0' && buff[digits] >= '0' && buff[digits] <= '0' + 9)
    {
        temp = temp * 10;
        digits++;
    }
    // 个 位有效 temp=10 digits=1
    // 十 位有效 temp=100 digits=2

    sizeT res = 0;

    for (sizeT i = 0; i < digits; i++)
    {
        temp = temp / 10;
        res = res + (buff[i] - '0') * temp;
    }

    return sign ? -res : res;
}
sizeT ftos_(char *buff, double f, sizeT precision)
{
    // 处理整数部分
    sizeT in = f;
    f = f - in;           // 小数部分
    in = itos_(buff, in); // 现在in 表示已经填充完成的位置!

    buff[in] = '.';

    sizeT temp = 0;
    if (f < 0)
    {
        f = -f;
    }

    for (sizeT i = 1;; i++)
    {
        in++;
        f = f * 10;
        temp = f;
        buff[in] = '0' + temp;

        f = f - temp;
        if (precision != 0 && i >= precision)
        {
            break;
        }

        if (precision == 0 && f == 0 || i > 6)
        {
            break;
        }
    }
    buff[in + 1] = '\0';
    return in;

    // 处理小数部分
}

sizeT uitos_(char *buff, sizeT i)
{

    // 记录正负
    char flag = 0; // 0表示正
    if (i < 0)
    {
        flag = 1;
        i = -i;
    }

    // 获取位数
    sizeT digits = 1;
    sizeT temp = i;
    while (temp = temp / 10)
    {
        digits++;
    }

    if (flag != 0)
    {
        buff[0] = '-';
        buff = buff + 1;
    }
    buff[digits] = '\0';
    for (sizeT j = 0; j < digits; j++)
    {
        buff[digits - j - 1] = '0' + (i % 10);
        i = i / 10;
    }
    if (flag != 0)
    {
        return digits + 1;
    }

    return digits;
}
sizeT itos_(char *buff, int32 i)
{

    // 记录正负
    char flag = 0; // 0表示正
    if (i < 0)
    {
        flag = 1;
        i = -i;
    }

    // 获取位数
    sizeT digits = 1;
    sizeT temp = i;
    while (temp = temp / 10)
    {
        digits++;
    }

    if (flag != 0)
    {
        buff[0] = '-';
        buff = buff + 1;
    }
    buff[digits] = '\0';
    for (sizeT j = 0; j < digits; j++)
    {
        buff[digits - j - 1] = '0' + (i % 10);
        i = i / 10;
    }
    if (flag != 0)
    {
        return digits + 1;
    }

    return digits;
}

sizeT sprintf_(char *str, const char *format, ...)
{
    // 逐个参数往str中拷贝

    va_list args;           // 定义参数
    va_start(args, format); // 初始化
    char buff[64];          // 缓冲区 数值转字符串缓冲区

    int32 i = 0;
    int32 l = 0;
    while (format[i] != '\0')
    {
        if (format[i] != '%')
        {
            str[l] = format[i];
        }
        else
        {
            char f = format[i + 1];

            switch (f)
            {
            case 'd':
                i++;
                itos_(buff, va_arg(args, sizeT));
                l = l - 1 + strcpy_(str + l, buff);
                // d ==整数
                break;
            case 'f':
                i++;
                // f ==小数
                double tempI = va_arg(args, double);
                ftos_(buff, tempI, 0);
                l = l - 1 + strcpy_(str + l, buff);
                break;
            case 's':
                i++;
                l = l - 1 + strcpy_(str + l, va_arg(args, char *));
                // s == 字符串
                break;
            case 'c':
                i++;
                // todo 24/1/3 去不出char类型!!!
                str[l] = (char)va_arg(args, int);
                // c == 字符
                break;
            case 'p':
                i++;
                uitos_(buff, va_arg(args, ui64));
                l = l - 1 + strcpy_(str + l, buff);
                // d ==整数
                break;
            default:
                // 说明%后并非有效格式
                str[l] = format[i];
                break;
            }
        }
        i++;
        l++;
    }
    str[l] = '\0';
    va_end(args);
    return l;
}

void memset_(char *str, char a, sizeT n)
{
    for (sizeT i = 0; i < n; i++)
    {
        str[i] = a;
    }
}