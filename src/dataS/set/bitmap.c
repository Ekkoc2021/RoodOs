/*
 * @Author: EKKO 189890049@qq.com
 * @Date: 2023-12-31 15:55:30
 * @LastEditors: EKKO 189890049@qq.com
 * @LastEditTime: 2024-01-16 23:49:01
 * @Description: 位图:1有效,0无效
 *      清除某个位
 *      设置某个位
 *      查找第一已经被设置的位
 *      查找最后一个已经被设置的位
 *      查找第一个没有被设置的位
 *      查找最后一个没有被设置的位
 *      清空所有位
 *      测试位:返回某个位是否已经被设置
 *
 */
#include "bitmap.h"

#ifdef BITMAP_DEBUG
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
void tr(Bitmap *bmap)
{
    unsigned char *d = bmap->bits;

    // 遍历bitmap情况
    unsigned char temp;
    unsigned char mask;
    for (int i = 0; i < (bmap->size) >> 3; i++)
    {
        temp = d[i];

        /*
        0000 1000
        1111 1111
        */
        printf("%3d:", i);
        for (int x = 0; x < 8; x++)
        {
            printf("%d  ", x + 1);
        }
        printf("\n");
        printf("    ");
        for (int j = 0; j < 8; j++)
        {
            temp = (temp & (1 << j)) >> j;
            printf("%d  ", temp);
            temp = d[i];
        }
        printf("\n\n");
    }
}
int main(int argc, char const *argv[])
{
    // Bitmap b;
    // char *bits = malloc(200);
    // initBitmap(&b, 200, bits);
    // clearBitmap(&b);
    // // tr(&b);
    // printf("---------\n");
    // setBit(&b, 7);
    // setBit(&b, 6);
    // setBit(&b, -7);
    // setBit(&b, 227);
    // setBit(&b, 30);
    // for (int i = 0; i < 64; i++)
    // {
    //     setBit(&b, 0 + i);
    // }

    // // tr(&b);

    // clearBit(&b, 12);
    // clearBit(&b, 23);
    // clearBit(&b, 34);
    // clearBit(&b, 42);
    // clearBit(&b, 28);
    // clearBit(&b, 29);
    // clearBit(&b, 30);
    // clearBit(&b, 31);
    // for (int i = 0; i < 10; i++)
    // {
    //     setBit(&b, 10 + i);
    // }
    // // tr(&b);
    // printf("---------\n");
    // for (int i = 0; i < 8; i++)
    // {
    //     printf("%d  ", testBit(&b, 0 + i));
    // }
    // printf("---------\n");
    // for (int i = 0; i < 64; i++)
    // {
    //     setBit(&b, 0 + i);
    // }

    // for (int i = 0; i < 80; i++)
    // {
    //     setBit(&b, 0 + i);
    // }
    // clearBit(&b, 0);
    // // tr(&b);
    // printf("\n第一个为0的位置:%d ", find_fist_bit(&b));
    // printf("\n最后一个为0的位置:%d  \n", find_last_bit(&b));
    // setBit(&b, 0);
    // for (int i = 0; i < 10; i++)
    // {
    //     clearBit(&b,   i);
    // }
    // tr(&b);
    // printf("\n第一个为0的位置:%d  \n", find_fist_bit(&b));
    // printf("\n最后一个为0的位置:%d  ", find_last_bit(&b));
    // printf("\n第一个为1的位置:%d  ", find_fist_set_bit(&b));
    // printf("\n最后一个为1的位置:%d  ", find_last_set_bit(&b));

    Bitmap b1;
    char *bits1 = malloc(1024 * 64); // 2000*8=16000 16000 * 4kb
    // 32m为一个bitmap 32m / 4kb =8mb/1kb=8196 / 8=1024
    // 16GB/32mb =1GB /2mb = 1024 / 2 =512
    // 512 * 1024==>一次遍历大约0.015s
    // 128 * 1024==>一次遍历大约0.006s
    // 单从平均时间来看,下面的计算平均时间程序,短时间内基本不可能跑出来,但4GB的却在30s左右跑完了
    initBitmap2(&b1, 1024 * 64 * 8 - 28, bits1);
    clock_t start, end;
    double cpu_time_used;

    // 记录开始时间
    start = clock();

    // 遍历一次要花费多长时间?
    // 200 个字节
    for (int i = 0; i < b1.size; i++)
    {
        setBit(&b1, i);
    } // 全为1

    // 记录结束时间
    end = clock();

    // 计算经过的CPU时间
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

    printf("遍历一次花费的时间: %f 秒\n", cpu_time_used);

    // 记录开始时间
    start = clock();
    for (int i = 0; i < b1.size; i++)
    {
        clearBit(&b1, i);
        int ffb;
        int flb;
        flb = find_last_bit(&b1);
        ffb = find_fist_bit(&b1);

        if (i != ffb || i != flb)
        {
            printf("查找为0位置失败:位置为:%d ffb:%d flb:%d\n", i, ffb, flb);
        }
        setBit(&b1, i);
    }

    // 记录结束时间
    end = clock();

    // 计算经过的CPU时间
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

    printf("查找总花费的时间: %f 秒\n", cpu_time_used);
    printf("平均花费的时间: %f 秒\n", cpu_time_used / b1.size);
    //--------
    for (int i = 0; i < b1.size; i++)
    {
        clearBit(&b1, i);
    } // 全为1

    for (int i = 0; i < b1.size; i++)
    {
        setBit(&b1, i);

        int flsb = find_last_set_bit(&b1);
        int ffsb = find_fist_set_bit(&b1);

        if (i != flsb || i != ffsb)
        {
            printf("查找为0位置失败:位置为:%d flsb:%d ffsb:%d\n", i, flsb, ffsb);
        }
        clearBit(&b1, i);
    }

    return 0;
}

#endif
void initBitmap(Bitmap *bitmap, bMLengthType Bytes, char *data)
{
    bitmap->bits = data;
    bitmap->size = Bytes * 8;
    bitmap->used = 0;
}

// 1/16 size 不一定就是8的整数倍
void initBitmap2(Bitmap *bitmap, bMLengthType size, char *data)
{
    bitmap->bits = data;
    bitmap->size = size;
    bitmap->used = 0;
}

void setBit(Bitmap *bitmap, bMLengthType position)
{
    if (position < 0 && position >= bitmap->size)
    {
        return;
    }
    unsigned char *d = bitmap->bits;
    // 7 % 8=7 8%8=0 14 % 8=6
    unsigned char set = 1 << (position % 8);
    bMLengthType index = position >> 3;
    (bitmap->bits)[index] = ((bitmap->bits)[index]) | set;
    //   1111 1010
    // | 0000 0001
    //   1111 1011
    bitmap->used++;
}
void clearBit(Bitmap *bitmap, bMLengthType position)
{
    if (position < 0 && position >= bitmap->size)
    {
        return;
    }
    unsigned char *d = bitmap->bits;
    // 7 % 8=7 8%8=0 14 % 8=6
    unsigned char set = ~(1 << (position % 8));
    bMLengthType index = position >> 3;
    (bitmap->bits)[index] = ((bitmap->bits)[index]) & set;
    //   1111 1010
    // & 1111 0111
    //   1111 0010
    bitmap->used--;
}
void clearBitmap(Bitmap *bitmap)
{
    bitmap->used = 0;
    unint32 *b = (unint32 *)(bitmap->bits);
    // 一次清理32位
    bMLengthType length = (bitmap->size >> 5);
    for (int i = 0; i < (bitmap->size >> 5); i++)
    {
        b[i] = 0;
    }
    // 防止没有清干净
    int x = bitmap->size % (8 * 4); // 还有x个没有清理
    for (int i = 0; i < (x >> 3); i++)
    {
        (bitmap->bits)[length * 4 - 1 + i] = 0;
    }
}

char testBit(Bitmap *bitmap, bMLengthType position)
{

    if (position < 0 || position >= bitmap->size)
    {
        return 1;
    }
    unsigned char loc = (bitmap->bits)[position >> 3];
    char re = (loc & (1 << (position % 8))) >> (position % 8);
    return re;
}

bMLengthType find_fist_bit(Bitmap *bitmap)
{
    unint32 *b = (unint32 *)(bitmap->bits);
    // 从index位开始找
    bMLengthType index = (bitmap->size >> 5) << 2;
    // 以4B为单位搜索
    for (int i = 0; i < (bitmap->size >> 5); i++)
    {
        if (b[i] - 0xFFFFFFFF != 0)
        {
            index = i * 4;
            break;
        }
    }

    bMLengthType l = 0;
    if (bitmap->size % 8 == 0)
    {
        l = bitmap->size >> 3;
    }
    else
    {
        l = (bitmap->size >> 3) + 1;
    }
    // 缩小范围从index开始找
    for (int i = index; i < l; i++)
    {
        // 以B为单位搜索
        if (((bitmap->bits)[i] - 0xFF) != 0)
        {
            // 以bit为单位继续搜索
            for (int j = 0; j < 8; j++)
            {
                // 就用写好的testBit,也就多了两条运算
                if (!testBit(bitmap, (i * 8 + j)))
                {
                    if (i * 8 + j >= bitmap->size)
                    {
                        return -1;
                    }

                    return i * 8 + j;
                }
            }
        }
    }
    return -1;
}

bMLengthType find_last_bit(Bitmap *bitmap)
{
    unint32 *b = (unint32 *)(bitmap->bits);
    // 从index位开始找
    bMLengthType index = (bitmap->size >> 5) << 2;
    bMLengthType l = 0;
    if (bitmap->size % 8 == 0)
    {
        l = (bitmap->size >> 3) - 1;
    }
    else
    {
        l = bitmap->size >> 3;
    }

    //-----todo 1/1:抽成函数?
    for (int i = l; index - 1 < i; i--)
    {
        if ((bitmap->bits)[i] != 0xFF)
        {
            for (int j = 7; j >= 0; j--)
            {
                if (i * 8 + j < bitmap->size && !testBit(bitmap, (i * 8 + j)))
                {
                    return i * 8 + j;
                }
            }
        }
    }
    // 以4B为单位反向搜索
    for (int i = 0; i < (bitmap->size >> 5); i++)
    {
        if (b[i] - 0xFFFFFFFF != 0)
        {
            index = i * 4;
            break;
        }
    }

    for (int i = (bitmap->size >> 3) - 1; index - 1 < i; i--)
    {
        if ((bitmap->bits)[i] != 0xFF)
        {
            for (int j = 7; j >= 0; j--)
            {
                if (i * 8 + j < bitmap->size && !testBit(bitmap, (i * 8 + j)))
                {
                    return i * 8 + j;
                }
            }
        }
    }
    return -1;
}

bMLengthType find_fist_set_bit(Bitmap *bitmap)
{
    unint32 *b = (unint32 *)(bitmap->bits);
    // 从index位开始找
    bMLengthType index = (bitmap->size >> 5) << 2;
    // 以4B为单位搜索
    for (int i = 0; i < (bitmap->size >> 5); i++)
    {
        if (b[i] != 0) // 说明有1
        {
            index = i * 4;
            break;
        }
    }
    // 缩小范围从index开始找
    bMLengthType l = 0;
    if (bitmap->size % 8 == 0)
    {
        l = bitmap->size >> 3;
    }
    else
    {
        l = (bitmap->size >> 3) + 1;
    }
    for (int i = index; i < l; i++)
    {
        // 以B为单位搜索
        if ((bitmap->bits)[i] != 0) // 说明有1
        {
            // 以bit为单位继续搜索
            for (int j = 0; j < 8; j++)
            {
                // 就用写好的testBit,也就多了两条运算
                if (testBit(bitmap, i * 8 + j))
                {
                    if (i * 8 + j < bitmap->size)
                    {
                        return i * 8 + j;
                    }
                }
            }
        }
    }
    return -1;
}

bMLengthType find_last_set_bit(Bitmap *bitmap)
{
    unint32 *b = (unint32 *)(bitmap->bits);
    // 从index位开始找
    bMLengthType index = (bitmap->size >> 5) << 2;

    bMLengthType l = 0;
    if (bitmap->size % 8 == 0)
    {
        l = (bitmap->size >> 3) - 1;
    }
    else
    {
        l = bitmap->size >> 3;
    }
    //-----todo 1/1:抽成函数?
    // 反向搜索4B为单位搜索的盲区
    for (int i = l; index - 1 < i; i--)
    {
        if ((bitmap->bits)[i] != 0) // 有1
        {
            for (int j = 7; j >= 0; j--)
            {
                if (i * 8 + j < bitmap->size && testBit(bitmap, (i * 8 + j)))
                {

                    return i * 8 + j;
                }
            }
        }
    }
    // 以4B为单位反向搜索
    for (int i = 0; i < (bitmap->size >> 5); i++)
    {
        if (b[i] != 0) // 有1
        {
            index = i * 4;
            break;
        }
    }

    for (int i = (bitmap->size >> 3) - 1; index - 1 < i; i--)
    {
        if ((bitmap->bits)[i] != 0) // 有1
        {
            for (int j = 7; j >= 0; j--)
            {
                if (i * 8 + j < bitmap->size && testBit(bitmap, (i * 8 + j)))
                {
                    return i * 8 + j;
                }
            }
        }
    }
    return -1;
}
