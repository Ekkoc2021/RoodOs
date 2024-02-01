/*
 * @Author: EKKO 189890049@qq.com
 * @Date: 2023-12-29 13:02:06
 * @LastEditors: EKKO 189890049@qq.com
 * @LastEditTime: 2024-01-03 14:12:56
 * @Description: 链表的实现==>循环链表
 */

#include "linkedList.h"
//----------------------TEST------------------------------
#ifdef LINKEDLIST_DEBUG
#include <stdlib.h>
#include <stdio.h>
char compareTo(void *a, void *b)
{
    if (*(int *)a - *(int *)b > 0)
    {
        return 1;
    }
    else if (*(int *)a - *(int *)b < 0)
    {
        return -1;
    }
    return 0;
}
void tr(linkedList *list);
int main(int argc, char const *argv[])
{
    linkedList list;
    initLinkedList(&list);

    for (int i = 0; i < 10; i++)
    {
        int *x = malloc(sizeof(int));
        *x = i;
        listNode *p = malloc(sizeof(listNode));
        p->data = x;
        add(&list, p);
        /* code */
    }
    tr(&list);
    printf("-----------------\n");

    int a;
    a = 1;
    delete (&list, &a, compareTo);
    a = 2;
    delete (&list, &a, compareTo);
    a = 6;
    delete (&list, &a, compareTo);
    tr(&list);
    listNode *p = malloc(sizeof(listNode));
    int d = 100;
    p->data = &d;
    insert(&list, p, 1);
    printf("--------\n");
    tr(&list);
    printf("hello world!\n");
    return 0;
}

void tr(linkedList *list)
{
    listNode *temp = list->head.next;
    while (temp != &(list->head))
    {

        printf("%d\n", *(int *)(temp->data));
        temp = temp->next;
    }
}

#endif
//---------------------------------------------------------

void mergeList(linkedList *dest, linkedList *source)
{
    listNode *temp;
    while (1)
    {
        temp = deleteIn(source, 0);
        if (temp == &(source->head))
        {
            return;
        }
        add(dest, temp);
    }
}

// 初始化 链表
void initLinkedList(linkedList *list)
{
    listNode *head = &(list->head);
    head->next = head;
    head->pre = head;
    list->length = 0;
}

// 尾部添加节点
void add(linkedList *list, listNode *node)
{

    node->pre = list->head.pre;
    list->head.pre->next = node;
    list->head.pre = node;
    node->next = &(list->head);
    list->length++;
}

// 删除节点
listNode *delete(linkedList *list, void *data, char (*compareTo)(void *A, void *B))
{
    listNode *temp = list->head.next;
    while (temp != &(list->head))
    {
        if ((*compareTo)(temp->data, data) == 0)
        {
            // => => =>
            // <= <= <=
            listNode *f = temp->pre;
            // =>   =>
            // <=   <=
            f->next = temp->next;
            f->next->pre = f;
            list->length--;
            break;
        }
        temp = temp->next;
    }
    return temp;
}
listNode *deleteIn(linkedList *list, linkdeLengthType index)
{

    listNode *temp = list->head.next;
    if (index < 0 || index >= list->length)
    {
        return temp;
    }
    linkdeLengthType i = 0;
    while (temp != &(list->head))
    {
        if (i == index)
        {
            // => => =>
            // <= <= <=
            listNode *f = temp->pre;
            // =>   =>
            // <=   <=
            f->next = temp->next;
            f->next->pre = f;
            list->length--;
            break;
        }
        i++;
        temp = temp->next;
    }
    return temp;
}
// 查找接口
listNode *search(linkedList *list, void *data, char (*compareTo)(void *A, void *B))
{
    listNode *temp = list->head.next;
    while (temp != &(list->head))
    {

        if ((*compareTo)(temp->data, data) == 0)
        {
            break;
        }
        temp = temp->next;
    }
    return temp;
}

int insert(linkedList *list, listNode *n, int index)
{
    if (index + 1 > list->length)
    {
        add(list, n);
        return list->length - 1;
    }

    listNode *pre = &(list->head);
    for (int i = 0; i < index; i++)
    {
        pre = pre->next;
    }

    n->pre = pre;
    n->next = pre->next;
    pre->next->pre = n;
    pre->next = n;
    list->length++;
    return index;
}

void deleteNode(linkedList *list, listNode *node)
{
    // => => =>
    // <= <= <=
    listNode *f = node->pre;
    // =>   =>
    // <=   <=
    f->next = node->next;
    f->next->pre = f;
    list->length--;
}
