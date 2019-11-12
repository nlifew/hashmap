

#ifndef _UTIL_RBDBG_H
#define _UTIL_RBDBG_H 1

#include <memory.h>

#include "list.h"
/*
void cat_node_map(struct rb_node *node)
{
    struct rb_node *i = node;
    if (i == NULL) {
        printf("[i][0, NUL][NUL]\n");
        return;
    }
    else
        printf("[i][%d, %s][%p]\n", i->hash, i->color == RB_BLK ? "BLK" : "RED", i);
    
    if ((i = node->part) == NULL)
        printf("[p][0, NUL][NUL]\n");
    else
        printf("[p][%d, %s][%p]\n", i->hash, i->color == RB_BLK ? "BLK" : "RED", i);
    
    if ((i = node->left) == NULL)
        printf("[l][0, NUL][NUL]\n");
    else
        printf("[l][%d, %s][%p]\n", i->hash, i->color == RB_BLK ? "BLK" : "RED", i);

    if ((i = node->right) == NULL)
        printf("[r][0, NUL][NUL]\n");
    else
        printf("[r][%d, %s][%p]\n", i->hash, i->color == RB_BLK ? "BLK" : "RED", i);
}
*/



static void cat_rbtree(struct rb_node *root)
{
    /** 按照深度把当前的树打印出来
      * 这里为了美观，使用了两个链表(其实是当作队列使用)
      */

    struct list list_array[2];
    memset(list_array, 0, sizeof(list_array));

    struct list *cur_list = list_array, *next_list = list_array + 1;
    struct list_node *list_node;
    struct rb_node *node;

    add_list_tail(cur_list, root, 0);
    while (1) {
        /** 不断从当前链表中取出首个元素
          * 如果不是空，那么把它的左右子节点加入到另一个链表的尾部
          */
        list_node = remove_list_head(cur_list);
        if (list_node != NULL) {
            node = (struct rb_node*) (list_node->value);
            if (node == NULL) {
                printf("[NUL] ");
            }
            else {
                // printf("[%d][%s][%X] ", node->hash, node->color == RB_RED ? 
                //     "RED" : "BLK", (int) node);
                printf("[%d][%s] ", node->hash, node->color == RB_RED ? "RED" : "BLK");
                add_list_tail(next_list, node->left, 0);
                add_list_tail(next_list, node->right, 0);
            }
            free(list_node);
        }
        else if (next_list->ls_head != NULL) {
            /** 当前链表为空，我们检查另一个链表
              * 如果它不为空，那么需要切换当前链表的指针
              */
            printf("\n");
            struct list *tmp = cur_list;
            cur_list = next_list;
            next_list = tmp;
        }
        else {
            /** 两个链表全部为空，说明树遍历完成
              */
            printf("\n");
            break;
        }
    }
}

static void do_test_rbtree(struct rb_node *node, struct list *list, int *blk, int *lines)
{
    struct list_node *list_node;

    if (node != NULL) {

        /** 当前节点不是空，那么递归遍历，直到找到 NIL 
          * 为了得到递归的路径，我们把当前节点加到链表的尾部
          */
        add_list_tail(list, node, 0);
        do_test_rbtree(node->left, list, blk, lines);
        do_test_rbtree(node->right, list, blk, lines);

        if ((list_node = remove_list_tail(list)) != NULL)
            free(list_node);
        return;
    }

    // 如果到达这里，说明我们已经得到了一条从根节点到 NIL 的路径
    // 路径存储在 list 中

    *lines = (*lines) + 1;

    int black = 0;
    struct rb_node *e, *ee;

    // 打印出当前路径
    /*
    for (list_node = list->ls_head; list_node; list_node = list_node->next) {
        e = (struct rb_node*) (list_node->value);
        printf("%d[%s][%p]->", e->hash, e->color == RB_RED ? "RED" : "BLK", e);
    }
    printf("[NUL]\n");
*/

    for (list_node = list->ls_head; list_node; list_node = list_node->next) {
        e = (struct rb_node*) (list_node->value);
        if (e->color == RB_BLK) {
            black ++;
        }

        if (list_node->next != NULL) {
            ee = (struct rb_node*)(list_node->next->value);

            // 检查是否有两个红节点直接连接
            if (e->color == RB_RED && ee->color == RB_RED) {
                printf("two RED node linked\n");
            }

            if (ee == e->left) {
                // 检查左子节点的值是否比当前节点小
                if (ee->hash >= e->hash) {
                    printf("the hash of left node is greater\n");
                }
                // 检查左子节点的父节点是否是当前节点
                if (ee->part != e) {
                    printf("the part of left is NOT me\n");
                }
            }
            else if (ee == e->right) {
                // 检查右子节点的值是否比当前节点大
                if (ee->hash <= e->hash) {
                    printf("the hash of right is smaller\n");
                }
                // 检查右子节点的父节点是否是当前节点
                if (ee->part != e) {
                    printf("the part of right is NOT me\n");
                }
            }
            else {
                printf("broken stack\n");
            }
        }
    }

    // 检查是否经过了同样的黑节点
    if (*blk == 0) {
        *blk = black;
    }
    else if (*blk != black) {
        printf("invalid black node %d, last is %d\n", black, *blk);
    }
}

static void test_rbtree(struct rb_node *root)
{
    // printf("-------------------------------------------------------------------->\n");
    // cat_rbtree(root);

    struct list list;
    memset(&list, 0, sizeof(list));


    // 检验根节点是否为黑色
    if (root != NULL && root->color == RB_RED) {
        printf("The root of rbtree is red\n");
    }

    int rb_blk = 0, lines = 0;
    do_test_rbtree(root, &list, &rb_blk, &lines);

    // printf("for %d lines\n", lines);
}

#endif
