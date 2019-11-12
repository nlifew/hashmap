

#ifndef _UTIL_RBTREE_H
#define _UTIL_RBTREE_H 1

enum
{
    RB_RED = 0,
    RB_BLK = 1,   
};

struct rb_node
{
    void *key;
    void *value;
    int hash;
    int color;
    struct rb_node *left;
    struct rb_node *right;
    struct rb_node *part;
};

void free_rbtree(struct rb_node *root);

struct rb_node* new_rb_node(const void *key, int hash, 
    const void *val, size_t val_t);

struct rb_node* get_rbtree2(struct rb_node *root, 
    const void *key, int hash, int (*cmp)(const void*, const void*));

struct rb_node* put_rbtree(struct rb_node **root, 
    struct rb_node *new_node, int (*cmp)(const void*, const void*));

struct rb_node* remove_rbtree2(struct rb_node **root,
    const void *key, int hash, int (*cmp)(const void*, const void*));

#endif