/*
 * @Author: EKKO 189890049@qq.com
 * @Date: 2023-12-29 09:53:02
 * @LastEditors: EKKO 189890049@qq.com
 * @LastEditTime: 2024-01-02 23:41:17
 * @Description: 或许还有一些其他的问题!至少目前(2023-12-29)测试还算正常
 *  2024-01-01 满足使用要求!
 */

#ifndef RBTREE_H_1229
#define RBTREE_H_1229
#define BLACK 0
#define RED 1
// #define RB_TREE_DEBUG   // 调试
typedef int RBTLenType; // 定义长度类型!
typedef struct RBNode RBNode;
typedef struct RBTree RBTree;
// 能轻松获取父亲节点,能表示颜色,能存储数据
struct RBNode
{
    char color;
    void *data;
    RBNode *p;
    RBNode *left;
    RBNode *right;
};

// char (*compareTo)(void *A, void *B) 接口
// 红黑树内部比较大小会调用此方法,A的逻辑值大于B返回值大于0,等于返回0,小于返回值小于0

struct RBTree
{
    RBNode *Nil;
    RBNode *root; // Nil中的两个域可以利用起来表示根,或者长度!!!
    char (*compareTo)(void *A, void *B);
    RBTLenType length; // 长度感觉可以是不必要的的
};

/**
 * @description: 初始化一棵红黑树,tree,nil,以及比较的函数
 *          存放的数据在逻辑上可比较的
 *          char compareTo(void *dataA,void *dataB),传入数据能够比较
 *          dataA大于dataB返回值小于0,大于返回大于0,等于返回0
 *          应该需要注意的是,返回类型为char,注意返回值溢出问题!
 *          dataA一定是RBNode的data!
 * @param RBTree  *tree :树
 * @param RBNode  *nil : 空节点
 * @param char  (*compareTo)(void *A, void *B) :compareTo是这样类型的函数 char compareTo(void *dataA,void *dataB)函数
 * @return
 */
void initRBTree(RBTree *tree, RBNode *nil, char (*compareTo)(void *A, void *B));

// 插入操作
/**
 * @description: 红黑树节点的插入
 * @param RBTree *tree
 * @param RBNode *node
 * @return   1 成功,0 失败
 */
char insertRBT(RBTree *tree, RBNode *node);

// 查找操作
/**
 * @description: 查找节点
 * @param RBTree *tree
 * @param void *data 要查找的数据,
 * @return  * 返回已经搜索到的节点数据
 */
RBNode *searchRBT(RBTree *tree, void *data);

/**
 * @description: 删除操作,根据传入的数据,而非Node节点,最终会返回一个Node
 *          红黑树只负责维护节点的逻辑结构,不负责管理节点销毁
 * @param RBTree *tree
 * @param void *data 删除的数据的地址!
 * @return  返回删除的节点的数据,根据需要处理该节点,红黑树不负责销毁节点
 */
RBNode *deleteRBT(RBTree *tree, void *data);

/**
 * @description: 查找红黑树中的最大值
 * @param RBTree *tree 对应的红黑树
 * @return * 返回值最大的节点,若不存在返回nil节点!
 */
RBNode *getMaximum(RBTree *tree);

/**
 * @description: 查找最小值
 * @param RBTree *tree
 * @param RBNode *root
 * @return * 返回值最小的节点,若不存再返回nil节点!
 */
RBNode *getMinimum(RBTree *tree);

/**
 * @description: 找到最小值节点,删除,并返回该节点!
 * @param {RBTree} *tree
 * @return {*}
 */
RBNode *delete_min(RBTree *tree);

/**
 * @description: 找到最大值节点,删除并返回!
 * @param {RBTree} *tree
 * @return {*}
 */
RBNode *delete_max(RBTree *tree);

RBNode *deleteNodeRBT(RBTree *tree, RBNode *node);

#endif