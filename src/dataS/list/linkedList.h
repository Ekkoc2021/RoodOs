/*
 * @Author: EKKO 189890049@qq.com
 * @Date: 2023-12-30 23:09:32
 * @LastEditors: EKKO 189890049@qq.com
 * @LastEditTime: 2024-01-03 14:16:33
 * @Description: 双向循环链表!
 */

// #define LINKEDLIST_DEBUG
#ifndef LINKEDLIST_H_1230
#define LINKEDLIST_H_1230

typedef int linkdeLengthType;
typedef struct LinkedListNode listNode;
typedef struct LinkedList linkedList;
// 考虑前驱
struct LinkedListNode
{
    listNode *pre;  // 前驱
    listNode *next; // 后继
    void *data;     // 数据域
};

// 能够轻松找到尾节点==>循环列表!
struct LinkedList
{
    listNode head; // head->pre 尾节点
    linkdeLengthType length;
};

/**
 * @description: 删除下标为index的节点
 * @param {linkedList} *list
 * @param {void} *data
 * @param {linkdeLengthType} index
 * @return {*}
 */
listNode *deleteIn(linkedList *list, linkdeLengthType index);
/**
 * @description: 初始化链表
 * @return {*}
 */
void initLinkedList(linkedList *list);

/**
 * @description: 添加节点
 * @param {linkedList} *list
 * @param {node} *node
 * @return {*}
 */
void add(linkedList *list, listNode *node);

/**
 * @description: 删除数据为data的元素
 *       按照给的比较接口进行对比:char compareTo(void * dataA, void *dataB);
 *      dataA为对应节点的data域,dataB随意,返回dataA-dataB
 *      A大于B返回值大于0,等于返回0,小于返回值小0
 * @param {linkedList} *list
 * @param {void} *data
 * @param {char} *compareTo
 * @return {*}
 */
listNode *delete(linkedList *list, void *data, char (*compareTo)(void *A, void *B));

/**
 * @description: 按照data搜索,同样需要提供compareTo接口
 * @param {linkedList} *list
 * @param {void} *data
 * @param {char} *compareTo
 * @return {*}
 */

listNode *search(linkedList *list, void *data, char (*compareTo)(void *A, void *B));
/**
 * @description: 节点插入,在索引为index的位置插入,返回插入的位置,index>length,插入到尾部
 * @param {linkedList} *list
 * @param {node} *n
 * @param {int} index
 * @return {*}
 */
int insert(linkedList *list, listNode *n, int index);

/**
 * @description: 确保node属于list,直接删除node
 * @param {linkedList} *list
 * @param {node} *n
 */
void deleteNode(linkedList *list, listNode *node);

/**
 * @description: 合并list,将source中的节点合并到dest
 * @param {linkedList} *list
 * @param {node} *n
 */
void mergeList(linkedList *dest, linkedList *source);
#endif