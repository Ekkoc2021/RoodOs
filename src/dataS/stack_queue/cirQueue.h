#ifndef CIRQUEUE_2412
#define CIRQUEUE_2412
// #define CIRQUEUE_DEBUG

#include "../list/arrayList.h"

typedef struct cirQueue
{
    arrayList data;
    AListLenType size;  // 已经使用的容量!!!,不是长度!!
    AListLenType front; // 头指针位置
    AListLenType tail;  // 尾指针
} cirQueue;
// 初始化队列
void initCQueue(cirQueue *q, void *da, eSizeType eSize, AListLenType len);

// 入队操作
char cirEnqueue(cirQueue *queue, void *elem);

// 出队操作
char cirDequeue(cirQueue *queue, void *elem);

// 查看队头元素,返回队头元素的data域,请确保队列长度不为空!
char cirFront(cirQueue *queue, void *elem);

// 查看队尾元素
char cirRear(cirQueue *queue, void *elem);

// 判空操作
char cirIsEmpty(cirQueue *queue);

// 是否为满
char cirIsFull(cirQueue *queue);
#endif