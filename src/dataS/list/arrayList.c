/*
 * @Author: EKKO 189890049@qq.com
 * @Date: 2023-12-29 12:31:32
 * @LastEditors: EKKO 189890049@qq.com
 * @LastEditTime: 2024-01-03 13:08:47
 * @Description: 线性表==>数组实现
 */
#include "arrayList.h"
// 需要排序吗?
#ifdef ARRARYLIST_DEBUG
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char const *argv[])
{
    srand(time(NULL));
    printf("hello arrarylist!\n");
    arrayList a1;
    /* code */
    char *d = malloc(20 * sizeof(char));
    initArList(&a1, d, sizeof(char), 20);

    char randomNumber;
    char *getTest;
    for (int i = 0; i < 20; i++)
    {
        randomNumber = rand() % 100;
        setElem(&a1, &randomNumber, i);
        getElem(&a1, &getTest, i);
        printf("%d ", *getTest);
        if (randomNumber != *getTest)
        {
            printf("i=%d 添加数据=%d", i, randomNumber);
            return 0;
        }
    }
    printf("\n");
    for (int i = 0; i < 20; i++)
    {
        getElem(&a1, &getTest, i);
        printf("%d ", *getTest);
    }

    return 0;
}
#endif

void initArList(arrayList *l, void *da, eSizeType eSize, AListLenType len)
{
    l->data = da;
    l->elemSize = eSize;
    l->length = len;
}

// 汇编专门指令实现会更加高效
void memCopy(void *destination, void *source, eSizeType size)
{
    char *d = (char *)destination;
    char *s = (char *)source;
    for (eSizeType i = 0; i < size; i++)
    {
        d[i] = s[i];
    }
}

char setElem(arrayList *list, void *element, AListLenType index)
{
    if (index >= list->length || index < 0)
    {
        return 0; // 超出范围 插入失败!
    }

    AListLenType i = index * (list->elemSize);

    char *data = (char *)list->data;

    memCopy(data + i, element, list->elemSize);

    return 1;
}

char getElem(arrayList *list, void **element, AListLenType index)
{
    if (index >= list->length || index < 0)
    {
        return 0; // 超出范围
    }

    AListLenType i = index * (list->elemSize);

    char *data = (char *)list->data;

    *element = data + i;

    return 1;
}

AListLenType getByCmp(arrayList *list, char (*compareTo)(void *A, void *B))
{
    if (list->length == 0)
    {
        return list->length + 1;
    }

    char *arr = (char *)(list->data);
    char *target = arr;
    char *targetIndex = 0;
    for (AListLenType i = 1; i < list->length; i++)
    {
        if (compareTo(arr + i * (list->elemSize), target) > 0)
        {
            target = arr + i * (list->elemSize);
            targetIndex = i;
        }
    }
    return targetIndex;
}