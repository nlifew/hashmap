

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "include/rbtree.h"


static void left_rotate(struct rb_node *node);
static void right_rotate(struct rb_node *node);
static void replace_node(struct rb_node *old_node, struct rb_node *new_node);
static struct rb_node* balance_insert(struct rb_node *root, struct rb_node *new_node);
static struct rb_node* balance_remove(struct rb_node *root, struct rb_node *old_node);


struct rb_node* get_rbtree2(struct rb_node *root, 
    const void *key, int hash, int (*cmp_func)(const void*, const void*))
{
    struct rb_node *node = root;
    int cmp;

    while (node) {
        if ((cmp = hash - node->hash) == 0 && 
            (cmp = cmp_func(key, node->key)) == 0) {
            break;
        }
        node = cmp < 0 ? node->left : node->right;
    }
    return node;
}


struct rb_node* put_rbtree(struct rb_node **root, 
    struct rb_node *new_node, int (*cmp_func)(const void*, const void*))
{
    struct rb_node *node, *next = *root;

    int cmp;
    const int hash = new_node->hash;
    const void *key = new_node->key;

    while ((node = next) != NULL) {
        if ((cmp = hash - node->hash) == 0 && 
            (cmp = cmp_func(key, node->key)) == 0) {
            replace_node(node, new_node);
            break;
        }
        else if (cmp < 0 && (next = node->left) == NULL)
            node->left = new_node;
        else if (cmp > 0 && (next = node->right) == NULL)
            node->right = new_node;

        new_node->part = node;
    }
    *root = balance_insert(*root, new_node);
    return node;
}

static struct rb_node* balance_insert(struct rb_node *root, struct rb_node *new_node)
{
    struct rb_node *i = new_node, *p, *pp, *u;

    do {
        // 如果没有父节点，说明是根节点，直接涂黑
        if ((p = i->part) == NULL) {
            i->color = RB_BLK;
            root = i;
            break;
        }

        // 如果父节点是黑色，直接怼上去
        if (i->color == RB_BLK || p->color == RB_BLK) {
            break;
        }

        // 此时父节点是红色，而根节点是黑色，因此肯定存在祖父节点
        pp = p->part;
        u = pp->left == p ? pp->right : pp->left;

        // 叔叔节点为红色，涂黑父节点和叔叔节点，
        // 涂红祖父节点，并以祖父节点为当前节点
        if (u != NULL && u->color == RB_RED) {
            p->color = u->color = RB_BLK;
            pp->color = RB_RED;
            i = pp;
            continue;
        }

        if (p == pp->left) {
            if (i == p->right) {
                left_rotate(p);
                i = p;
                p = i->part;
                pp = p->part;
            }
            right_rotate(pp);
        }
        else {
            if (i == p->left) {
                right_rotate(p);
                i = p;
                p = i->part;
                pp = p->part;
            }
            left_rotate(pp);
        }
        p->color = RB_BLK;
        pp->color = RB_RED;
        i = p;
    } while (1);

    return root;
}


struct rb_node* remove_rbtree2(struct rb_node** root, 
    const void *key, int hash, int (*cmp_func)(const void*, const void*))
{
    struct rb_node* old_node = get_rbtree2(*root, key, hash, cmp_func);
    if (old_node == NULL) {
        return NULL;
    }

    // 找到替换节点
    struct rb_node *node = NULL, *next;
    if ((next = old_node->right) != NULL) {
        do {
            node = next;
        } while ((next = next->left) != NULL);
    } 
    else if ((next = old_node->left) != NULL) {
        /** 只有左子节点时，可以根据红黑树的性质推理出：
          * old_node 为黑色
          * 左子节点为红色
          * 左子节点的两个子节点都是 NIL
          */
        node = next;
    }
    else if (old_node->part == NULL) {
        // 如果要移除根节点，直接返回
        *root = NULL;
        return old_node;
    }
    else {
        // 看来这个 old_node 也是个没有孩子的单身狗
        node = old_node;
    }
    
    *root = balance_remove(*root, node);

    // 此时，old_node 可能移动到了根节点的位置
    // 我们需要重置下根节点
    if (old_node->part == NULL)
        *root = node;

    // 把替换节点从红黑树中移除
    // *注意* 此时替换节点是可能存在子节点的
    if ((next = node->left) != NULL)
        next->part = node->part;
    else if ((next = node->right) != NULL)
        next->part = node->part;
    if (node->part == NULL) {
    }
    else if (node->part->left == node)
        node->part->left = next;
    else 
        node->part->right = next;
    node->part = node->left = node->right = NULL;

    // 用替换节点替换掉老节点
    replace_node(old_node, node);

    return old_node;
}


static struct rb_node* balance_remove(struct rb_node *root, struct rb_node *old_node)
{
    struct rb_node *r = old_node, *p, *s, *sl, *sr;
    do {
        sl = sr = NULL;
        // 如果是红节点或根节点，直接返回
        if (r->color == RB_RED || (p = r->part) == NULL) {
            r->color = RB_BLK;
            break;
        }
        // 当前节点为父节点的左节点
        if (p->left == r) {
            if ((s = p->right) != NULL && s->color == RB_RED) {
                s->color = RB_BLK;
                p->color = RB_RED;
                left_rotate(p);
                if (s->part == NULL)
                    root = s;
                s = p->right;
            }
            sl = s == NULL ? NULL : s->left;
            sr = s == NULL ? NULL : s->right;

            // 兄弟节点的右节点为黑色
            if (sr == NULL || sr->color == RB_BLK) {
                // 左节点也是黑色
                if (sl == NULL || sl->color == RB_BLK) {
                    if (s != NULL) 
                        s->color = RB_RED;
                    r = p;
                    continue;
                }
                // 此时兄弟节点的左节点为红色，因此 s 不为空
                s->color = RB_RED;
                sl->color = RB_BLK;
                right_rotate(s);
                s = sl;
                sl = s->left;
                sr = s->right;
                // cat_node_map(s);
            }
            // 此时兄弟节点的右节点为红色
            s->color = p->color;
            p->color = RB_BLK;
            sr->color = RB_BLK;
            left_rotate(p);
            if (s->part == NULL)
                root = s;
        }
        else {
            if ((s = p->left) != NULL && s->color == RB_RED) {
                s->color = RB_BLK;
                p->color = RB_RED;
                right_rotate(p);
                if (s->part == NULL)
                    root = s;
                s = p->left;
            }
            sl = s == NULL ? NULL : s->left;
            sr = s == NULL ? NULL : s->right;

            if (sl == NULL || sl->color == RB_BLK) {
                if (sr == NULL || sr->color == RB_BLK) {
                    if (s != NULL)
                        s->color = RB_RED;
                    r = p;
                    continue;
                }
                s->color = RB_RED;
                sr->color = RB_BLK;
                left_rotate(s);
                s = sr;
                sl = s->left;
                sr = s->right;
            }
            s->color = p->color;
            p->color = RB_BLK;
            sl->color = RB_BLK;
            right_rotate(p);
            if (s->part == NULL)
                root = s;
        }
        break;
    } while(1);

    return root;
}

/**
  *      1                2
  *       \              /
  *        2     -->    1
  *       /              \
  *      3                3
  * 
  * 需要改动的部分：
  * 0 的子节点，3 的父节点
  * 1 的右节点，1 的父节点
  * 2 的父节点，2 的左节点
  */
static void left_rotate(struct rb_node *node) 
{
    struct rb_node *one, *two, *three, *zero;
    one = node;
    two = node->right;
    zero = node->part;
    three = two == NULL ? NULL : two->left;

    if (zero == NULL) {
    }
    else if (zero->left == one)
        zero->left = two;
    else 
        zero->right = two;

    if (three != NULL)
        three->part = one;

    if (two != NULL) {
        two->part = zero;
        two->left = one;
    }

    one->right = three;
    one->part = two;
}

static void right_rotate(struct rb_node *node)
{
    struct rb_node *one, *two, *three, *zero;
    two = node;
    zero = node->part;
    one = node->left;
    three = one == NULL ? NULL : one->right;

    if (zero == NULL) {
    }
    else if (zero->left == two)
        zero->left = one;
    else 
        zero->right = one;

    if (three != NULL)
        three->part = two;

    if (one != NULL) {
        one->part = zero;
        one->right = two;
    }
    two->part = one;
    two->left = three; 
}


static void replace_node(struct rb_node *old_node, struct rb_node *new_node)
{
    if (new_node != NULL) {
        new_node->part = old_node->part;
        new_node->left = old_node->left;
        new_node->right = old_node->right;
        new_node->color = old_node->color;
    }

    struct rb_node *node;

    if ((node = old_node->left) != NULL)
        node->part = new_node;
    if ((node = old_node->right) != NULL)
        node->part = new_node;
    if ((node = old_node->part) == NULL) {
    }
    else if (node->left == old_node)
        node->left = new_node;
    else 
        node->right = new_node;
}


struct rb_node* new_rb_node(const void *key, 
    int hash, const void *val, size_t val_t)
{
    const size_t mem_t = sizeof(struct rb_node) + val_t;
    struct rb_node *node = (struct rb_node*) malloc(mem_t);
    if (node == NULL) {
        return NULL;
    }

    node->value = val_t == 0 ? (void*) val : memcpy(node + 1, val, val_t);

    node->hash = hash;
    node->key = (void*) key;
    node->color = RB_RED;
    node->left = node->right = node->part = NULL;

    return node;
}

void free_rbtree(struct rb_node *root)
{
    if (root == NULL) {
        return;
    }
    free_rbtree(root->left);
    free_rbtree(root->right);
    free(root);
}


