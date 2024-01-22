/*
 * @Author: EKKO 189890049@qq.com
 * @Date: 2023-12-31 13:39:49
 * @LastEditors: EKKO 189890049@qq.com
 * @LastEditTime: 2023-12-31 14:19:18
 * @Description: 链队的实现:双向链表实现
 */
#include "linkedQueue.h"

#ifdef QUEUE_DEBUG
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char const *argv[])
{
    linkedQueue q;
    initQueue(&q);

    if (isEmpty(&q))
    {
        printf("队列为空!\n");
    }
    for (int i = 0; i < 6; i++)
    {
        int *x = malloc(sizeof(int));
        *x = i + 98;
        queueNode *p = malloc(sizeof(queueNode));
        p->data = x;
        enqueue(&q, p);
    }
    if (!isEmpty(&q))
    {
        printf("队列不为空!\n");
        printf("队头:%d,队尾%d \n", *(int *)front(&q), *(int *)rear(&q));
    }
    int size = q.size;
    for (int i = 0; i < size; i++)
    {
        printf("%d ,", *(int *)(dequeue(&q)->data));
    }
    printf("\n");
    for (int i = 0; i < 10; i++)
    {
        int *x = malloc(sizeof(int));
        *x = i + 200;
        queueNode *p = malloc(sizeof(queueNode));
        p->data = x;
        enqueue(&q, p);
    }
    if (!isEmpty(&q))
    {
        printf("队列不为空!\n");
        printf("队头:%d,队尾%d \n", *(int *)front(&q), *(int *)rear(&q));
    }
    size = q.size;
    for (int i = 0; i < size; i++)
    {
        printf("%d ,", *(int *)(dequeue(&q)->data));
    }
    printf("\n");
    if (isEmpty(&q))
    {
        printf("队列为空!\n");
    }

    return 0;
}
#endif
// 初始化队列
void initQueue(linkedQueue *q)
{
    q->head.pre = &(q->head);
    q->head.next = &(q->head);
    q->size = 0;
}

// 入队操作:队尾进入
void enqueue(linkedQueue *queue, queueNode *node)
{
    node->pre = queue->head.pre;
    queue->head.pre->next = node;
    node->next = &(queue->head);
    queue->head.pre = node;
    queue->size++;
}

// 出队操作:队头出
queueNode *dequeue(linkedQueue *queue)
{
    queueNode *d = &(queue->head);
    if (queue->size > 0)
    {
        d = d->next;
        queue->head.next = d->next;
        d->next->pre = &(queue->head);
        queue->size--;
    }
    return d;
}
void *front(linkedQueue *queue)
{
    return queue->head.next->data;
}

void *rear(linkedQueue *queue)
{
    return queue->head.pre->data;
}
char isEmpty(linkedQueue *queue)
{
    return queue->size == 0 ? 1 : 0;
}