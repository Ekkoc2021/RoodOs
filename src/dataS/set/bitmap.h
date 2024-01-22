/*
 * @Author: EKKO 189890049@qq.com
 * @Date: 2024-01-01 12:22:38
 * @LastEditors: EKKO 189890049@qq.com
 * @LastEditTime: 2024-01-01 12:27:16
 * @Description: bitmap 接口
 */
// #define BITMAP_DEBUG
#ifndef BITMAP_H2411
#define BITMAP_H2411
typedef unsigned int bMLengthType;
typedef unsigned int unint32;

typedef struct
{
    unsigned char *bits; // 1设置,0为没有设置
    bMLengthType used;   // 已经设置长度
    bMLengthType size;   // 尚未设置的长度,位域的数量
} Bitmap;

void initBitmap(Bitmap *bitmap, bMLengthType length, char *data);
void initBitmap2(Bitmap *bitmap, bMLengthType size, char *data);
void setBit(Bitmap *bitmap, bMLengthType position);
void clearBit(Bitmap *bitmap, bMLengthType position);
// position位为1返回1,为0返回0,不在范围内返回1
char testBit(Bitmap *bitmap, bMLengthType position);
void clearBitmap(Bitmap *bitmap);
bMLengthType find_fist_bit(Bitmap *bitmap);
bMLengthType find_last_bit(Bitmap *bitmap);
bMLengthType find_fist_set_bit(Bitmap *bitmap);
bMLengthType find_last_set_bit(Bitmap *bitmap);
#endif