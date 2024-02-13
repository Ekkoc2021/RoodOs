/*
 * @Author: EKKO 189890049@qq.com
 * @Date: 2024-01-02 21:11:46
 * @LastEditors: EKKO 189890049@qq.com
 * @LastEditTime: 2024-01-03 00:03:55
 * @Description: 循环队列
 *  方案1: (v)
    头指针指向第一个节点,尾指针永远指向尾节点的下一个
    尾指针与头指针相等时,队列为空
    尾指针+1 等于头的为满

    方案2:
    头指针指向第一个节点的前一个
    尾指向最后一个
    尾等于头为空
    尾+1等于头,为满
    确定位置 尾节点的下一个位置:tail= (tail+1) % l
         头节点的下一个位置:front =(front+1) % l
     取出:q[++front] 加入q[++tail]=xx

 */
#include "cirQueue.h"

//---------------TEST-------------------------
#ifdef CIRQUEUE_DEBUG
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    printf("hello circular queue!\n");
    cirQueue c1;
    cirQueue *c = &c1;
    char temp;

    char *d = malloc(20 * sizeof(char));
    initCQueue(c, d, sizeof(char), 20);
    // 如何测试
    // 添加20个元素,遍历20个元素,查看尾和头的位置是否正确
    for (int i = 0; i < 21; i++)
    {
        temp = i;
        cirEnqueue(c, &temp);
    }
    for (int i = 0; i < 20; i++)
    {
        getElem(&(c1.data), &temp, i);
        printf(" %d:%d ", i, temp);
    }
    char f;
    char t;
    cirFront(c, &f);
    cirRear(c, &t);
    printf("\n头:%d,值%d,尾:%d,值%d,长度:%d\n", c1.front, f, c1.tail, t, c1.size);

    // 入队2个查看位置
    temp = 100;
    cirEnqueue(c, &temp);
    cirEnqueue(c, &temp);
    for (int i = 0; i < 20; i++)
    {
        getElem(&(c1.data), &temp, i);
        printf(" %d:%d ", i, temp);
    }
    cirFront(c, &f);
    cirRear(c, &t);
    printf("\n头:%d,值%d,尾:%d,值%d,长度:%d\n", c1.front, f, c1.tail, t, c1.size);

    // 出队3个查看位置!
    cirDequeue(c, &temp);
    printf(" %d\n ", temp);
    cirDequeue(c, &temp);
    printf(" %d\n ", temp);
    cirDequeue(c, &temp);
    printf(" %d\n ", temp);
    for (int i = 0; i < 20; i++)
    {
        getElem(&(c1.data), &temp, i);
        printf(" %d:%d ", i, temp);
    }
    cirFront(c, &f);
    cirRear(c, &t);
    printf("\n头:%d,值%d,尾:%d,值%d,长度:%d\n", c1.front, f, c1.tail, t, c1.size);

    // 再入队4个查看位置!
    temp = 100;
    cirEnqueue(c, &temp);
    cirEnqueue(c, &temp);
    cirEnqueue(c, &temp);
    cirEnqueue(c, &temp);
    for (int i = 0; i < 20; i++)
    {
        getElem(&(c1.data), &temp, i);
        printf(" %d:%d ", i, temp);
    }
    cirFront(c, &f);
    cirRear(c, &t);
    printf("\n头:%d,值%d,尾:%d,值%d,长度:%d\n", c1.front, f, c1.tail, t, c1.size);

    return 0;
}
#endif
//---------------TEST-END---------------------
// 初始化队列
void initCQueue(cirQueue *q, void *da, eSizeType eSize, AListLenType len)
{
    initArList(&(q->data), da, eSize, len);
    q->size = 0;
    q->front = 0;
    q->tail = 0;
}

// 判空操作
char cirIsEmpty(cirQueue *queue)
{
    if (queue->front == queue->tail)
    {
        return 1;
    }
    return 0;
}

// 是否为满
char cirIsFull(cirQueue *queue)
{
    AListLenType a = (queue->tail + 1) % (queue->data.length);
    if (a == queue->front)
    {
        return 1;
    }
    return 0;
}

// 入队操作
char cirEnqueue(cirQueue *queue, void *elem)
{
    if (cirIsFull(queue))
    {
        return 0;
    }
    setElem(&(queue->data), elem, queue->tail);
    queue->tail = (queue->tail + 1) % (queue->data.length);
    queue->size++;
    return 1;
}

// 出队操作
char cirDequeue(cirQueue *queue, void **elem)
{
    if (cirIsEmpty(queue))
    {
        return 0;
    }
    getElem(&(queue->data), elem, queue->front);
    queue->front = (queue->front + 1) % (queue->data.length);
    queue->size--;
    return 1;
}

// 查看队头元素
char cirFront(cirQueue *queue, void **elem)
{
    if (cirIsEmpty(queue))
    {
        return 0;
    }

    getElem(&(queue->data), elem, queue->front);
    return 1;
}

// 查看队尾元素
char cirRear(cirQueue *queue, void **elem)
{
    if (cirIsEmpty(queue))
    {
        return 0;
    }
    getElem(&(queue->data), elem, ((queue->front) + queue->size - 1) % (queue->data.length));
    return 1;
}