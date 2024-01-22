/*
 * @Author: EKKO 189890049@qq.com
 * @Date: 2023-12-30 23:45:41
 * @LastEditors: EKKO 189890049@qq.com
 * @LastEditTime: 2023-12-31 14:22:35
 * @Description: 栈数据结构
 */
// #define STACK_DEBUG
#ifndef STACK_H_1230
#define STACK_H_1230

typedef struct LinkedStack linkedStack;
typedef struct stackNode stackNode;
typedef int stackLengthType;

struct stackNode
{
    stackNode *next; // 后继
    void *data;      // 数据域
};

struct LinkedStack
{
    stackLengthType length;
    stackNode head; // head->pre 尾节点
};

/**
 * @description: 初始化栈,传入一个栈
 * @param {linkedStack} *s
 * @return {*}
 */
void initStack(linkedStack *s);

/**
 * @description: 出栈,若为空弹出s->head
 * @param {linkedStack} *s
 * @return {*}
 */
stackNode *pop(linkedStack *s);

/**
 * @description: 入栈操作
 * @param {linkedStack} *s
 * @param {stackNode} *n 入栈的节点
 * @return {*}
 */
void *push(linkedStack *s, stackNode *n);

/**
 * @description: 栈是否为空,是返回1,否返回0
 * @param {linkedStack} *s
 * @return {*}
 */
char isEmpty(linkedStack *s);

/**
 * @description: 查看栈顶元素
 * @param {linkedStack} *s
 * @return {*} 栈顶节点的data域
 */
void *getTop(linkedStack *s);

#endif
