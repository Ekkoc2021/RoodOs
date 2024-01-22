/*
 * @Author: EKKO 189890049@qq.com
 * @Date: 2023-12-23 15:47:34
 * @LastEditors: EKKO 189890049@qq.com
 * @LastEditTime: 2024-01-01 21:49:40
 * @Description: 通用红黑树数据结构的实现!
 *  2023-12-26 23:50 基本功能已经完成!但是还不敢用!
 *  从原理到实现花了好多时间!
 *  调试是个难题,可能还有bug!
 *  2024-01-01 21:45 :
 *   通过批量的测试!解决了很多潜在的小问题,我以为删除和增加两个接口的bug最多,结果这两个接口反而是写得最好的!
 *   搜索函数的实现有问题!T_T 这种简单的接口出问题是最难想到的!一直报段错误!
 *   测试用比较接口,溢出也排查了好久!T_T
 *   终于可以放心用了,应该不会有问题了 T_T
 */

#include "RBTree.h"

// 以root为根左旋转,要求root右树存在
void leftRotate(RBTree *tree, RBNode *root);
// 以root为根右旋转,要求root左树存在
void rightRotate(RBTree *tree, RBNode *root);
// 红黑树的3+4重构:传入son,以son,son的父亲以及爷爷进行一次3+4重构,一定是有效重构,事实上还是左右旋转
// 其中有一个是nil都会重构失败(保持原样)
void reconstructTreeFrom34(RBTree *tree, RBNode *son);

//---------------------调试代码!!-----------------------
#ifdef RB_TREE_DEBUG
#include <stdio.h>
#include <stdlib.h>
// 返回值大于0,则为高度,等于0则不是红黑树!
RBTLenType isRBTree(RBTree *tree, RBNode *root)
{
    // 检查黑高度 是否一致
    if (root == tree->Nil)
    {
        return 1;
    }

    // 检查是否和父亲颜色一样是红色
    if (root == tree->root)
    { // 是根节点,只要检查是否是红色即可
        if (root->color == RED)
        {
            return 0;
        }
    }
    else
    {
        // 不是根,检查与父亲颜色是否都为红色
        if (root->color == RED && root->color == root->p->color)
        {
            return 0;
        }
    }

    // 逻辑大小检查
    if (root->left != tree->Nil && (*(tree->compareTo))(root->data, root->left->data) < 0)
    {
        return 0;
    }

    if (root->right != tree->Nil && (*(tree->compareTo))(root->right->data, root->data) < 0)
    {
        return 0;
    }

    RBTLenType lh = isRBTree(tree, root->left);
    RBTLenType rh = isRBTree(tree, root->right);
    // 高度检查
    if (lh == 0 && rh == 0 && rh != lh)
    {
        return 0;
    }
    if (root->color == BLACK)
    {
        return rh + 1;
    }
}

char compareTo(void *a, void *b)
{
    // 1/1 直接返回 *(int *)a - *(int *)b 不妥当
    // 测试中a和b的值之差大于char能表示的范围溢出!!!!
    int t = (*(int *)a - *(int *)b);
    if (t > 0)
    {
        return 1;
    }
    else if (t < 0)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}
void LevelOrder();

int main(int argc, char const *argv[])
{
    RBTree tree1;
    tree1.compareTo = compareTo; // 以int类型作为测试

    RBNode nil = {.data = "null"};
    /*
    tree1.Nil = &nil;
    RBNode rt1;
    RBNode rt4 = {.color = RED, .left = &nil, .right = &nil, .data = &d};
    RBNode rt3 = {.color = RED, .left = &nil, .right = &rt4, .data = &c};
    RBNode rt2 = {.color = RED, .left = &nil, .p = &rt1, .right = &rt3, .data = &b};
    rt3.p = &rt2;
    rt4.p = &rt3;

    rt1.color = BLACK;
    rt1.data = &a;
    rt1.left = &nil;
    rt1.right = &rt2;
    rt1.p = &nil;
    tree1.root = &rt1;

    tree1.length = 4;

    tree1.root = &rt1;

    reconstructTreeFrom34(&tree1, &rt3);
    reconstructTreeFrom34(&tree1, &rt4);

    int s = 4;
    RBNode *my;
    if ((my = search(&tree1, &s)) != tree1.Nil)
    {
        printf("%d\n", *(int *)(my->data));
    }
    */

    //----------------
    /*
    RBTree tree; // 创建树
    // tree.Nil = &nil;
    // tree.root = &nil;
    // tree.compareTo = compareTo;
    initRBTree(&tree, &nil, compareTo);

    for (int i = 5; i < 10; i++)
    {
        RBNode *n = malloc(sizeof(RBNode));
        int *x = malloc(sizeof(int));
        *x = 20 - i;

        n->data = x;
        printf("a: %d \n", *(int *)(n->data));
        insert(&tree, n);
    }
    LevelOrder(&tree);
    for (int i = 5; i < 10; i++)
    {
        RBNode *n = malloc(sizeof(RBNode));
        int *x = malloc(sizeof(int));
        *x = i;
        n->data = x;
        printf("%d \n", *(int *)(n->data));
        insert(&tree, n);
    }
    RBNode *m = getMaximum(&tree);
    if (m != tree.Nil)
    {
        printf("最大值:%d\n", *(int *)(m->data));
    }

    LevelOrder(&tree);
    int de = 12;
    m = getMaximum(&tree);
    if (m != tree.Nil)
    {
        printf("最大值:%d\n", *(int *)(m->data));
    }
    m = getMinimum(&tree);
    if (m != tree.Nil)
    {
        printf("最小值:%d\n", *(int *)(m->data));
    }
    delete (&tree, &de);
    de = 8;
    m = getMaximum(&tree);
    if (m != tree.Nil)
    {
        printf("最大值:%d\n", *(int *)(m->data));
    }
    m = getMinimum(&tree);
    if (m != tree.Nil)
    {
        printf("最小值:%d\n", *(int *)(m->data));
    }
    delete (&tree, &de);
    LevelOrder(&tree);
    de = 15;
    m = getMaximum(&tree);
    if (m != tree.Nil)
    {
        printf("最大值:%d\n", *(int *)(m->data));
    }
    m = getMinimum(&tree);
    if (m != tree.Nil)
    {
        printf("最小值:%d\n", *(int *)(m->data));
    }
    delete (&tree, &de);
    LevelOrder(&tree);
    m = getMaximum(&tree);
    if (m != tree.Nil)
    {
        printf("最大值:%d\n", *(int *)(m->data));
    }
    m = getMinimum(&tree);
    if (m != tree.Nil)
    {
        printf("最小值:%d\n", *(int *)(m->data));
    }
    de = 14;
    delete (&tree, &de);
    LevelOrder(&tree);
    m = getMaximum(&tree);
    if (m != tree.Nil)
    {
        printf("最大值:%d\n", *(int *)(m->data));
    }
    m = getMinimum(&tree);
    if (m != tree.Nil)
    {
        printf("最小值:%d\n", *(int *)(m->data));
    }
    de = 6;
    delete (&tree, &de);
    LevelOrder(&tree);
    m = getMaximum(&tree);
    if (m != tree.Nil)
    {
        printf("最大值:%d\n", *(int *)(m->data));
    }
    m = getMinimum(&tree);
    if (m != tree.Nil)
    {
        printf("最小值:%d\n", *(int *)(m->data));
    }
    de = 62;
    delete (&tree, &de);
    LevelOrder(&tree);
    m = getMaximum(&tree);
    if (m != tree.Nil)
    {
        printf("最大值:%d\n", *(int *)(m->data));
    }
    m = getMinimum(&tree);
    if (m != tree.Nil)
    {
        printf("最小值:%d\n", *(int *)(m->data));
    }
    if (!isRBTree(&tree, tree.root))
    {
        printf("不是红黑树");
    }
    */
    //--------进行添加,删除测试-----------
    RBTree tree_test; // 创建树
    srand(time(NULL));
    // check insert
    RBNode *t;
    int *x;
    int a111;
    RBNode *n;

    for (int i = 0; i < 1000; i++)
    {

        initRBTree(&tree_test, &nil, compareTo);
        for (int j = 0; j < 50; j++)
        {
            n = malloc(sizeof(RBNode));
            n->left = NULL;
            n->right = NULL;
            x = malloc(sizeof(int));
            if (n == NULL || x == NULL)
            {
                printf(",%d \n", j);
                return 0;
            }

            *x = rand() % 250;
            n->data = x;
            // LevelOrder(&tree_test);
            insert(&tree_test, n);
            if (!isRBTree(&tree_test, tree_test.root))
            {
                LevelOrder(&tree_test);
                isRBTree(&tree_test, tree_test.root);
                printf("插入%d 测试失败!\n", *x);
                return 0;
            }
        }
        isRBTree(&tree_test, tree_test.root);
        for (int i = 0; i < 250; i++)
        {
            int a111 = 250;
            t = search(&tree_test, &a111);
            if (t != tree_test.Nil && *(int *)(t->data) != a111)
            {
                printf("搜索失败!");
                // LevelOrder(&tree_test);
            }
        }

        // LevelOrder(&tree_test);
        for (int i = 0; i < 50; i++)
        {
            a111 = rand() % 250;
            t = delete (&tree_test, &a111);
            if (!isRBTree(&tree_test, tree_test.root))
            {
                printf("随机删除:%d 测试失败!\n", a111);
                LevelOrder(&tree_test);
            }
            if (t != tree_test.Nil)
            {
                free(t->data);
                free(t);
            }
        }

        for (int i = 0; i < 215; i++)
        {
            t = delete_max(&tree_test);
            if (!isRBTree(&tree_test, tree_test.root))
            {
                printf("随机删除最大测试失败!\n");
                LevelOrder(&tree_test);
            }
        }

        for (int i = 0; i < 225; i++)
        {
            t = delete_min(&tree_test);
            if (!isRBTree(&tree_test, tree_test.root))
            {
                printf("随机删除最小测试失败!\n");
                LevelOrder(&tree_test);
            }
            if (t != tree_test.Nil)
            {
                free(t->data);
                free(t);
            }
        }
    }

    return 0;
}

// 一层一层输出RBtree节点
void LevelOrder(RBTree *tree)
{
    if (tree->root == tree->Nil)
    {
        printf("end!\n");
        return;
    }

    // 循环队列,最大长度要算上叶子节点
    int l = (tree->length) * 2;
    RBNode *q[l];

    // 方案1:
    // 头指针指向第一个节点,尾指针永远指向尾节点的下一个
    // 尾指针与头指针相等时,队列为空
    // 尾指针+1 等于头的为满

    // 方案2:
    // 头指针指向第一个节点的前一个
    // 尾指向最后一个
    // 尾等于头为空
    // 尾+1等于头,为满
    // 确定位置 尾节点的下一个位置:tail= (tail+1) % l
    //      头节点的下一个位置:front =(front+1) % l
    //  取出:q[++front] 加入q[++tail]=xx
    int front = 0, tail = 0;

    // 每层的尾指针
    // 先放入第一层的指针,层尾指针就指向第一个

    q[(tail + 1) % l] = tree->root;
    tail++;
    int end = tail;
    // 取出一个节点,不为nil则放入其后继节点
    // 直到是尾巴节点为止
    char temp;
    int data;
    while (front != tail)
    {
        while (front != end)
        {
            // 找到中轴线==>然后依次输出数据即可
            if (q[(front + 1) % l] != tree->Nil)
            {
                if (q[(front + 1) % l]->color == BLACK)
                {
                    temp = 'b';
                }
                else
                {
                    temp = 'r';
                }

                data = *(int *)(q[(front + 1) % l]->data);

                printf("   %c:%d     ", temp, data);

                q[(tail + 1) % l] = q[(front + 1) % l]->left;
                tail = (tail + 1) % l;
                q[(tail + 1) % l] = q[(front + 1) % l]->right;
                tail = (tail + 1) % l;
            }
            else
            {
                if (q[(front + 1) % l]->color == BLACK)
                {
                    temp = 'b';
                }
                else
                {
                    temp = 'r';
                }

                printf("   %c:Nil    ", temp);
            }
            front = (front + 1) % l;
        }
        end = tail;
        printf("\n");
    }
    printf("end!\n");
}

#endif

//----------------------------------------------------------------------------------

// 初始化一棵红黑树
void initRBTree(RBTree *tree, RBNode *nil, char (*compareTo)(void *A, void *B))
{
    tree->length = 0;
    tree->root = nil;
    tree->compareTo = compareTo;
    tree->Nil = nil;
}

// 寻找从某个节点的最大节点
RBNode *maximum(RBTree *tree, RBNode *root)
{

    RBNode *d = root;
    // root 为 nil 进不了循环,进了循环出去的方式只有return
    while (d != tree->Nil)
    {
        if (d->right != tree->Nil)
        {
            d = d->right;
        }
        else
        {
            return d;
        }
    }
    return tree->Nil;
}
// 寻找从某个节点开始的逻辑上最小节点
RBNode *minimum(RBTree *tree, RBNode *root)
{
    RBNode *d = root;
    while (d != tree->Nil)
    {
        if (d->left != tree->Nil)
        {
            d = d->left;
        }
        else
        {
            return d;
        }
    }
    return tree->Nil;
}
RBNode *findRealDelNode(RBTree *tree, RBNode *node)
{

    RBNode *d = maximum(tree, node->left);
    if (d != tree->Nil && d->color == RED)
    {
        return d; // 一定是红色
    }
    d = minimum(tree, node->right);
    if (d == tree->Nil)
    {
        return node; // 可能是红色,也可能不是!
    }
    return d; // 可能是红色,也可能不是!
}

// 传入一个p节点,并指明是否是左树比右树矮一截,然后进行调整!
void del_Adjust(RBTree *tree, RBNode *p, char isLeft)
{
    // 此时 总有一边矮一截==>调整
    RBNode *nep1; // 最远的那个侄子
    RBNode *nep2;
    RBNode *bro;
    while (0 == 0)
    {
        // 找到侄子,兄弟
        if (isLeft)
        {
            bro = p->right;
            nep1 = p->right->right;
            nep2 = p->right->left;
        }
        else
        {
            bro = p->left;
            nep1 = p->left->left;
            nep2 = p->left->right;
        }
        // 情况1调整前:
        // 情况1:最远侄子为红色,兄弟改父色,父改黑色,侄1变黑,以侄1进行34调整
        if (nep1->color == RED)
        {
            bro->color = p->color;
            p->color = BLACK;
            nep1->color = BLACK;
            reconstructTreeFrom34(tree, nep1);
            return; // 已经平衡直接返回
        }
        // 情况2:侄1黑,侄2红:侄2改黑,兄弟改红,以兄弟进行一次旋转
        if (nep2->color == RED)
        {
            bro->color = RED;
            nep2->color = BLACK;
            if (isLeft)
            {
                // 在左边!则nep2也左
                rightRotate(tree, bro);
            }
            else
            {
                leftRotate(tree, bro);
            }
        }

        else if (bro->color == RED && nep1->color == BLACK && nep2->color == BLACK)
        {
            //  2调整完原来后成为两侄都为黑,原来兄弟为红会进入情况3

            // 情况3:兄红两侄黑:父变红,兄变黑,往矮的方向进行旋转
            p->color = RED;
            bro->color = BLACK;
            if (isLeft)
            {
                leftRotate(tree, p);
            }
            else
            {
                rightRotate(tree, p);
            }
            // p还是没变,同时方向也没变
        }
        else if (bro->color == BLACK && nep1->color == BLACK && nep2->color == BLACK)
        {
            // 原父亲是黑色,还需要继续调整
            if (p->color == BLACK && p != tree->root)
            {
                // 唯一一种会跑到根的地方!

                // 如果p已经是根节点,兄弟是黑色的
                // p->p为Nil,会出错!
                // 原来删除节点为黑,且没有有效子节点
                // 两个侄子也为黑色的情况下,这棵没删除前就3个节点,且全是黑色
                // 兄弟改红色已经结束!,归为父亲为红的类处理

                // 正常就按照这里逻辑处理就行!
                bro->color = RED;
                // 得到新p,isLeft也要更新
                // 更新
                if (p == p->p->left)
                {
                    isLeft = 1;
                }
                else
                {
                    isLeft = 0;
                }
                p = p->p;
            }
            else
            {
                // 父红,修改完颜色后已经平衡
                p->color = BLACK;
                bro->color = RED;
                return;
            }
        }
    }
}
RBNode *delete(RBTree *tree, void *data)
{
    //
    RBNode *d1 = search(tree, data);
    if (d1 == tree->Nil)
    {
        return d1;
    }

    // findRealNode
    RBNode *d = findRealDelNode(tree, d1);

    if (d == tree->root)
    {
        tree->root = tree->Nil;
        return d;
    }

    // 替换数据
    void *temp = d1->data;
    d1->data = d->data;
    d->data = temp;
    // d是真正要删除的数据,d的两个子节点都是叶子节点
    //----------是红色
    if (d->color == RED)
    {
        if (d->p->left == d)
        {
            d->p->left = tree->Nil;
            return d;
        }
        d->p->right = tree->Nil;
        return d;
    }
    //-------是黑色,但是有一个红色子节点,这种情况已经被排除

    //-----是黑色,且没有任何叶子节点

    // 删除
    RBNode *p = d->p;

    char isLeft = 0; // 矮一截的是左还是右
    if (d->p->left == d)
    {
        d->p->left = tree->Nil;
        isLeft = 1;
    }
    else
    {
        d->p->right = tree->Nil;
    }
    //----------------------------------------
    del_Adjust(tree, p, isLeft); // 左右树不
    return d;

    // 会不会最终执行完后,根节点变红?所有情况,没有一个主动将p改为红色
}

RBNode *search(RBTree *tree, void *data)
{

    RBNode *d = tree->root;
    char compare;
    while (d != tree->Nil)
    {
        // 1/1 因为省略else,一直出现段异常,调试好久!
        char compare = (*tree->compareTo)(d->data, data);
        if (compare == 0)
        {
            return d;
        }
        else if (compare > 0)
        {
            d = d->left;
        }
        else
        {
            d = d->right;
        }
    }

    return d;
}

// 找到父亲的位置
RBNode *findLocation(RBTree *tree, void *data)
{
    RBNode *d = tree->root;
    while (d != tree->Nil)
    {

        if ((*(tree->compareTo))(d->data, data) > 0)
        {
            if (d->left == tree->Nil)
            {
                return d;
            }
            d = d->left;
        }
        else
        {
            if (d->right == tree->Nil)
            {
                return d;
            }
            d = d->right;
        }
    }
    return d;
}

char insert(RBTree *tree, RBNode *node)
{
    if (node == tree->Nil)
    {
        // 这种情况几乎不可能出现,返回值要求成功与否,暂且如此判断
        return 0;
    }

    // 初始化node,data的是否具有值需要判断吗?
    // todo:或许可以检查一下data是否为NULL!

    node->color = RED;
    node->left = tree->Nil;
    node->right = tree->Nil;
    node->p = tree->Nil;
    RBNode *p = findLocation(tree, node->data);

    // 树为空时!

    if (p == tree->Nil)
    {
        tree->root = node;
        node->color = BLACK;
        tree->length = 1;
        return 1;
    }

    tree->length = tree->length + 1;
    // 父亲不是空时
    if ((*tree->compareTo)(p->data, node->data) > 0)
    {
        p->left = node;
    }
    else
    {
        p->right = node;
    }
    node->p = p;
    if (p->color == BLACK)
    {
        return 1;
    }

    // 调整
    //  获取叔叔节点,爷爷节点
    RBNode *u;
    RBNode *gf;
    RBNode *temp;

    while (node != tree->root && node->p->color != BLACK)
    {
        // 初始化
        p = node->p;
        gf = p->p;
        if (p == gf->left)
        {
            u = gf->right;
        }
        else
        {
            u = gf->left;
        }
        // 情况1:34重构,重构后根变黑,根的左右子变红
        // 事实上只有一个儿子需要改为红t_t
        if (u->color == BLACK)
        {
            temp = gf->p;
            // 记住左右
            char isLeft = 0;
            if (temp->left == gf)
            {
                isLeft = 1;
            }

            // 统一处理的话,就只能在最后的时候处理颜色
            // 3个节点中最大的是那个!!!
            reconstructTreeFrom34(tree, node);
            if (temp == tree->Nil)
            {
                tree->root->color = BLACK;
                tree->root->left->color = RED;
                tree->root->right->color = RED;
                return 1;
            }
            if (isLeft)
            {
                temp->left->color = BLACK;
                temp->left->left->color = RED;
                temp->left->right->color = RED;
                return 1;
            }

            temp->right->color = BLACK;
            temp->right->left->color = RED;
            temp->right->right->color = RED;
            return 1;
        }

        // 情况2:叔叔为红:爷爷变红,父亲变黑,叔叔变黑
        if (u->color == RED)
        {
            gf->color = RED;
            p->color = BLACK;
            u->color = BLACK;
            node = gf;
        }
    }
    // 跳出循环有两种情况且执行到这里:node变为根节点,node的父亲是黑色!==>node一定是红色
    tree->root->color = BLACK;
    return 1;
}

RBNode *getMaximum(RBTree *tree)
{
    return maximum(tree, tree->root);
}

RBNode *getMinimum(RBTree *tree)
{
    return minimum(tree, tree->root);
}

void leftRotate(RBTree *tree, RBNode *root)
{
    if (root == tree->Nil || root->right == tree->Nil)
    {
        return;
    }

    RBNode *p = root->p;
    RBNode *son_right = root->right;
    RBNode *son_right_left = son_right->left;
    // 处理p与son的关系
    if (p != tree->Nil)
    {
        // 判断是root是左还是右
        if (root == p->left)
        {
            p->left = son_right;
        }
        else
        {
            p->right = son_right;
        }
        son_right->p = p;
    }
    else
    {
        son_right->p = tree->Nil;
        tree->root = son_right;
    }
    // root 与son的新关系
    root->p = son_right;
    son_right->left = root;

    // root的关系
    root->right = son_right_left;
    if (son_right_left != tree->Nil)
    {
        son_right_left->p = root;
    }
}

void rightRotate(RBTree *tree, RBNode *root)
{
    if (root == tree->Nil || root->left == tree->Nil)
    {
        return;
    }

    RBNode *p = root->p;
    RBNode *son_left = root->left;
    RBNode *son_left_right = son_left->right;

    // 处理p与son_left的关系
    if (p != tree->Nil)
    {
        // 判断是root是左还是右
        if (root == p->left)
        {
            p->left = son_left;
        }
        else
        {
            p->right = son_left;
        }
        son_left->p = p;
    }
    else
    {
        son_left->p = tree->Nil;
        tree->root = son_left;
    }

    // root 与son_left的新关系
    root->p = son_left;
    son_left->right = root;

    // root的关系
    root->left = son_left_right;
    if (son_left_right != tree->Nil)
    {
        son_left_right->p = root;
    }
}

// 红黑树的3+4重构:传入son,以son,son的父亲以及爷爷进行一次3+4重构,一定是有效重构,事实上还是左右旋转
// 其中有一个是nil都会重构失败(保持原样)
void reconstructTreeFrom34(RBTree *tree, RBNode *son)
{
    RBNode *nil = tree->Nil;
    if (son == nil || son->p == nil || son->p->p == nil)
    {
        return;
    }

    RBNode *a = son->p->p;

    RBNode *b = son->p;

    RBNode *c = son; // 不缺这几字节!

    char bMa = 0;
    if (b == a->right)
    {
        bMa = 1;
    }

    char cMb = 0;
    if (c == b->right)
    {
        cMb = 1;
    }

    // b > a ,c>b 则 R R
    if (bMa && cMb)
    {
        leftRotate(tree, a);
        return;
    }

    //  b>a ,c<b   则 R L
    if (bMa && !cMb)
    {
        rightRotate(tree, b);
        leftRotate(tree, a);
        return;
    }
    // b<a , c>b 则 L R
    if (!bMa && cMb)
    {
        leftRotate(tree, b);
        rightRotate(tree, a);
        return;
    }

    // b<a ,c<b 则 L L
    if (!bMa && !cMb)
    {
        rightRotate(tree, a);
        return;
    }
    // 总是感觉,我这34重构怪怪的:对7个元素排序后.....
}

RBNode *delete_min(RBTree *tree)
{
    if (tree->Nil == tree->root)
    {
        return tree->Nil;
    }

    RBNode *min = minimum(tree, tree->root);
    // min是根节点
    if (min == tree->root)
    {
        tree->root = tree->Nil;
        tree->length = 0;
        return min;
    }

    // min 不是根节点 一定没有左树
    min->p->left = min->right;
    if (min->right != tree->Nil)
    {
        min->right->p = min->p;
    }

    // min 是红色节点
    if (min->color == RED)
    {
        return min;
    }

    // 不是红色
    del_Adjust(tree, min->p, 1);
    return min;
}
RBNode *delete_max(RBTree *tree)
{

    if (tree->Nil == tree->root)
    {
        return tree->Nil;
    }

    RBNode *max = maximum(tree, tree->root);
    // min是根节点
    if (max == tree->root)
    {
        tree->root = tree->Nil;
        tree->length = 0;
        return max;
    }

    // max 不是根节点 一定没有右树
    max->p->right = max->left;
    if (max->right != tree->Nil)
    {
        max->left->p = max->p;
    }

    // min 是红色节点
    if (max->color == RED)
    {
        return max;
    }
    // 不是红色
    del_Adjust(tree, max->p, 0);
    return max;
}