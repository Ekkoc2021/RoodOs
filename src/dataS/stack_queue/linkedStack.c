/*
 * @Author: EKKO 189890049@qq.com
 * @Date: 2023-12-31 11:57:46
 * @LastEditors: EKKO 189890049@qq.com
 * @LastEditTime: 2023-12-31 14:02:40
 * @Description:
 */

#include "linkedStack.h"

//-------------------TEST----------------------------------
#ifdef STACK_DEBUG
#include <stdlib.h>
#include <stdio.h>
void tr(linkedStack *list);
int main(int argc, char const *argv[])
{
    linkedStack list;
    initStack(&list);
    if (isEmpty(&list))
    {
        printf("hello wolrd\n");
    }

    for (int i = 0; i < 10; i++)
    {
        int *x = malloc(sizeof(int));
        *x = i;
        stackNode *p = malloc(sizeof(stackNode));
        p->data = x;
        push(&list, p);
        /* code */
    }
    tr(&list);
    printf("-----------------\n");
    stackNode *p = malloc(sizeof(stackNode));
    int d = 100;
    p->data = &d;
    pop(&list);
    pop(&list);
    pop(&list);
    pop(&list);
    pop(&list);
    push(&list, p);
    printf("--------\n");
    tr(&list);
    printf("hello world stack top:%d\n", *(int *)getTop(&list));
    return 0;
}

void tr(linkedStack *s)
{
    stackNode *temp = s->head.next;
    while (temp != &(s->head))
    {

        printf("%d\n", *(int *)(temp->data));
        temp = temp->next;
    }
}

#endif
//---------------------------------------------------------
void initStack(linkedStack *s)
{
    s->head.next = &(s->head);
    s->length = 0;
}
stackNode *pop(linkedStack *s)
{
    stackNode *d = &(s->head);
    if (s->length > 0)
    {
        // => => =>
        // <= <= <=
        d = d->next;
        // =>   =>
        // <=   <=
        s->head.next = d->next;
        s->length--;
    }
    return d;
}

void *push(linkedStack *s, stackNode *n)
{
    n->next = s->head.next;
    s->head.next = n;
    s->length++;
}

char isEmpty(linkedStack *s)
{
    return s->length == 0 ? 1 : 0;
}

void *getTop(linkedStack *s)
{
    return s->head.next->data;
}
