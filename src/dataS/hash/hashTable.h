/*
 * @Author: EKKO 189890049@qq.com
 * @Date: 2024-01-03 14:06:09
 * @LastEditors: EKKO 189890049@qq.com
 * @LastEditTime: 2024-01-03 14:39:53
 * @Description:hash表接口
 *      需要创建一个hashtable数据类型
 *      通常需要创建一个hashNode的类型,然后让hashNode的data域指向你的data
 *      hashTable不会帮你管理内存!
 */

// #define HASHTABLE_DEBUG
#ifndef HASHTABL_2413
#define HASHTABL_2413
#include "../list/arrayList.h"
#include "../list/linkedList.h"
typedef listNode hashNode;
typedef struct
{
    arrayList table;
    AListLenType (*hash)(void *key); // hash 函数
    AListLenType length;             // 会不会超过表示范围?
} hashTable;

/**
 * @description: 初始化hashTable,注意length
 * @param {hashTable} *h
 * @param {AListLenType} length 为table数组的长度
 * @param {void} *table table 为linkedList类型的数组
 * @param {AListLenType} (*hash)(void *key) 一个能将你的data数据hash到长度以内的函数
 * @return {*}
 */
void initHashTa(hashTable *h, AListLenType length, void *table, AListLenType (*hash)(void *key));

/**
 * @description: 需要创建一个hashNode的类型,然后让hashNode的data域指向你的data,传入其地址!
 * @param {hashTable} *h hash表
 * @param {hashNode} *node 节点地址
 * @return {*}
 */
void insertion(hashTable *h, hashNode *node);

/**
 * @description: 删除具体某个key,需要传入一个能够比较hashnode中data域与传入key的函数
 * @param {hashTable} *h
 * @param {void} *key
 * @param {char}  (*compareTo)(void *A, void *B)
 *         一个能够比较hashnode中data域与传入key的函数
 *         A为data域的,B为传入的key的地址,返回值为 sgn(A-B)
 * @return {*}
 */
hashNode *deletion(hashTable *h, void *key, char (*compareTo)(void *A, void *B));

/**
 * @description: 查找
 * @param {hashTable} *h
 * @param {void} *key
 * @param {char}  (*compareTo)(void *A, void *B)
 * @return {*}
 */
hashNode *lookup(hashTable *h, void *key, char (*compareTo)(void *A, void *B));

/**
 * @description: 定位某个链表(通过key进行hash然后得到链表位置),删除并返回首个元素!
 * @param {hashTable} *h
 * @param {void} *key
 * @return {*}
 */
hashNode *popKey(hashTable *h, void *key);
#endif