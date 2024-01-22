/*
 * @Author: EKKO 189890049@qq.com
 * @Date: 2024-01-02 22:32:25
 * @LastEditors: EKKO 189890049@qq.com
 * @LastEditTime: 2024-01-03 23:29:59
 * @Description: 顺序表
 */
// #define ARRARYLIST_DEBUG
#ifndef ARRAYLIST_2412
#define ARRAYLIST_2412
typedef unsigned int AListLenType;
typedef unsigned short eSizeType;

// 长度,扩容
typedef struct
{
    eSizeType elemSize;  // 单个数据的长度!字节为单位!
    void *data;          // 数组地址
    AListLenType length; // 数组长度
} arrayList;

// 初始化arraylist
void initArList(arrayList *l, void *da, eSizeType eSize, AListLenType len);

// 基本的增删改查操作
char setElem(arrayList *list, void *element, AListLenType index);

// 获取元素
char getElem(arrayList *list, void **element, AListLenType index);

// 内存拷贝
void memCopy(void *destination, void *source, eSizeType size);

// 查找==>是否有序?
// AListLenType search(arrayList *list, char (*compareTo)(void *A, void *B));

// 按照比较函数查找最大值索引,最大值定义由compareTo给出,若没有找到将返回数组长度+1
AListLenType getByCmp(arrayList *list, char (*compareTo)(void *A, void *B));

#endif