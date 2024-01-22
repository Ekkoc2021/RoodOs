/*
 * @Author: EKKO 189890049@qq.com
 * @Date: 2023-12-31 13:39:57
 * @LastEditors: EKKO 189890049@qq.com
 * @LastEditTime: 2023-12-31 14:21:47
 * @Description: 链队接口
 */
// #define QUEUE_DEBUG
#ifndef LINKED_QUEUE_H_1231
#define LINKED_QUEUE_H_1231

typedef struct QueueNode queueNode;
typedef int queueSize;

struct QueueNode
{
    queueNode *pre;  // 前驱
    queueNode *next; // 后继
    void *data;      // 数据域
};
typedef struct
{
    queueSize size;
    queueNode head;
} linkedQueue;

// 初始化队列
void initQueue(linkedQueue *q);

// 入队操作
void enqueue(linkedQueue *queue, queueNode *node);

// 出队操作
queueNode *dequeue(linkedQueue *queue);

// 查看队头元素,返回队头元素的data域,请确保队列长度不为空!
void *front(linkedQueue *queue);

// 查看队尾元素
void *rear(linkedQueue *queue);

// 判空操作
char isEmpty(linkedQueue *queue);
#endif