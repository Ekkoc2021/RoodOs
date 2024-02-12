/*
 * @Author: EKKO 189890049@qq.com
 * @Date: 2024-01-03 10:37:41
 * @LastEditors: EKKO 189890049@qq.com
 * @LastEditTime: 2024-01-03 14:25:56
 * @Description: hash表的实现
 *      冲突解决:链表 1 2 4 8 16 32 64 128 256 512 1024 2048 ~
 *          搜索比较少或者几乎没有的情况下并不需要很高的查询速度!!
 */

#include "hashTable.h"

//---------------TEST-------------------
#ifdef HASHTABLE_DEBUG
#include <stdio.h>
#include <stdlib.h>

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

AListLenType myHash(void *elem)
{
    // elem 是一个linkedNode->data
    AListLenType e = *(AListLenType *)elem;

    int result = 0;
    int threshold = 1;

    while (e > threshold)
    {
        result++;
        threshold *= 2;

        if (threshold > 2048)
        {
            break;
        }
    }
    return result;
}

void traversalH(hashTable *h)
{
    linkedList *list;
    hashNode *temp;
    printf("hash表总长度为%d\n", h->length);
    for (int i = 0; i < h->table.length; i++)
    {
        getElem(&(h->table), &list, i);
        temp = list->head.next;
        printf("列表%d,长度为%d :", i, list->length);
        while (temp != &(list->head))
        {
            printf("  %d", *(int *)temp->data);
            temp = temp->next;
        }
        printf("\n");
    }
}
int main(int argc, char const *argv[])
{
    int a = 2148;
    printf("%d\n", myHash(&a));

    hashTable h1;
    hashTable *h = &h1;

    linkedList *list = malloc(13 * sizeof(linkedList));
    initHashTa(h, 13, list, myHash);

    hashNode *temp;
    int *tempd;
    // 是否会有丢失?
    for (int i = 0; i < 13; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            temp = malloc(sizeof(hashNode));
            tempd = malloc(sizeof(int)); // 数据均为int类型
            *tempd = (1 << i) + j;
            temp->data = tempd;
            insertion(h, temp);
        }
    }
    traversalH(h);
    printf("-----------------\n");
    tempd = malloc(sizeof(int));
    for (int i = 0; i < 13; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            *tempd = (1 << i) + 2;
            temp = lookup(h, tempd, compareTo);
            if (temp != 0x0)
            {
                printf("%d ", *(int *)temp->data);
            }
            else
            {
                printf("\n key:%d不存在\n", *tempd);
            }
        }
        printf("\n");
    }
    printf("-----------------\n");
    for (int i = 0; i < 13; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            *tempd = (1 << i) + 4;
            temp = deletion(h, tempd, compareTo);
            if (temp != 0x0)
            {
                printf("%d ", *(int *)temp->data);
            }
            else
            {
                printf("\n key:%d不存在\n", *tempd);
            }
        }
        printf("\n");
    }
    *tempd = 4096;
    temp = popKey(h, tempd);
    if (temp != 0x0)
    {
        printf("%d ", *(int *)temp->data);
    }
    else
    {
        printf("\n key:%d不存在\n", *tempd);
    }

    traversalH(h);
    return 0;
}
#endif

void initHashTa(hashTable *h, AListLenType length, void *table, AListLenType (*hash)(void *key))
{
    // 初始化 arraylist
    h->table.data = table;
    h->table.elemSize = sizeof(linkedList);
    h->table.length = length;

    // 初始化 hashTable的长度
    h->length = 0;
    h->hash = hash;

    // 初始化 所有的列表!
    linkedList *list;
    for (AListLenType i = 0; i < length; i++)
    {
        getElem(&(h->table), &list, i);
        initLinkedList(list);
    }
}

void insertion(hashTable *h, hashNode *node)
{
    AListLenType i = (*h->hash)(node->data);
    linkedList *list;
    getElem(&(h->table), &list, i);
    add(list, node);
    h->length++;
}

hashNode *deletion(hashTable *h, void *key, char (*compareTo)(void *A, void *B))
{

    // 获取到 arraylist的位置
    AListLenType i = (*h->hash)(key);
    // 拿到对应linkedList
    linkedList *list;
    getElem(&(h->table), &list, i);
    // 对linkedList 进行删除
    hashNode *d = delete (list, key, compareTo);
    if (d == &(list->head))
    {
        return 0x0;
    }
    h->length--;
    return d;
}
hashNode *lookup(hashTable *h, void *key, char (*compareTo)(void *A, void *B))
{
    // 获取到 arraylist的位置
    AListLenType i = (*h->hash)(key);
    // 拿到对应linkedList
    linkedList *list;
    getElem(&(h->table), &list, i);
    // 对linkedList 进行查找
    hashNode *d = search(list, key, compareTo);
    if (d == &(list->head))
    {
        return 0x0;
    }
    return d;
}

// 获取某个链表的第一个数据,返回并从表中删除,通过key定位链表
hashNode *popKey(hashTable *h, void *key)
{
    // 获取到 arraylist的位置
    AListLenType i = (*h->hash)(key);
    // 拿到对应linkedList
    linkedList *list;
    getElem(&(h->table), &list, i);
    hashNode *d = deleteIn(list, 0);
    if (d == &(list->head))
    {
        return 0x0;
    }
    h->length--;
    key = d->data;
    return d;
}