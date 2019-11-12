

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>


#include "include/hashmap.h"
#include "include/rbtree.h"

struct map_entry
{
    int size;
    struct rb_node *rbtree;
};

static int resize_hashmap(struct hash_map *map);
static void un_rbtree(struct rb_node **root);
static void to_rbtree(struct rb_node **root, int (*cmp)(const void*, const void*));

#define _MALLOC(t, n) (t*) malloc(sizeof(t) * n)
#define _REALLOC(t, p, n) (t*) realloc(p, sizeof(t) * n)
#define _IS_RBTREE(t) (t && t->color == RB_BLK)

struct hash_map* set_hashmap(struct hash_map *dst)
{
    struct hash_map *map = dst;
    if (map == NULL && (map = _MALLOC(struct hash_map, 1)) == NULL) {
        return NULL;
    }

    if (map->hm_cap == 0) 
        map->hm_cap = HASHMAP_DEF_CAPACITY;
    else {
        /* 将 capacity 调整为 2 的整次幂 */
        unsigned int n = map->hm_cap - 1;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        map->hm_cap = n + 1;
    }

    map->hm_size = 0;

    if (map->hm_load == 0) 
        map->hm_load = HASHMAP_DEF_LOAD_FACTOR;
    if (map->tree_t == 0)
        map->tree_t = HASHMAP_DEF_TREE_THRESHOLD;
    if (map->untr_t == 0)
        map->untr_t = HASHMAP_DEF_UNTREE_THRESHOLD;

    if (map->hm_cmp == NULL) {
        fprintf(stderr, "default compator selected\n");
        map->hm_cmp = HASHMAP_DEF_COMPARE;
    }
    if (map->hm_hash == NULL) {
        fprintf(stderr, "default hashcode selected\n");
        map->hm_hash = HASHMAP_DEF_HASHCODE;
    }


    if (map->hm_tab == NULL && (map->hm_tab = _MALLOC(struct map_entry, map->hm_cap)) == NULL) {
        fprintf(stderr, "failed to malloc hash_map table for %d capacity\n", map->hm_cap);
        if (dst != map) free(map);
        return NULL;
    }
    memset(map->hm_tab, 0, sizeof(struct map_entry) * map->hm_cap);

    return map;
}

void clear_hashmap(struct hash_map *map)
{
    if (map == NULL) {
        return;
    }

    map->hm_size = 0;

    for (int i = 0; i < map->hm_cap; i++) {
        struct map_entry *entry = map->hm_tab + i;
        struct rb_node *node = entry->rbtree;

        if (_IS_RBTREE(node)) {
            free_rbtree(node);
        }
        else {
            struct rb_node *next;
            while (node != NULL) {
                next = node->part;
                free(node);
                node = next;
            }
        }
        entry->size = 0;
        entry->rbtree = NULL;
    }
}

void free_hashmap(struct hash_map *map)
{
    if (map == NULL) {
        return;
    }

    clear_hashmap(map);
    free(map->hm_tab);

    // 保留 load_factor, tree_t, untr_t 
    map->hm_tab = NULL;
    map->hm_size = 0;
    map->hm_cap = 0;
}

void* get_hashmap(struct hash_map *map, const void *key)
{
    if (map == NULL) {
        return NULL;
    }

    // hashcode 和其高 16 位相异或，得到 hash

    int hash = map->hm_hash(key);
    hash ^= (unsigned int) hash >> 16;

    struct rb_node *node = map->hm_tab[hash & (map->hm_cap - 1)].rbtree;

    if (_IS_RBTREE(node)) {
        node = get_rbtree2(node, key, hash, map->hm_cmp);
    }
    else {
        while (node && (hash != node->hash || map->hm_cmp(key, node->key))) {
            node = node->part;
        }
    }
    return node ? node->value : NULL;
}

int put_hashmap(struct hash_map *map, const void *key, const void *val, size_t val_t)
{
    if (map == NULL) {
        return -1;
    }

    int hash = map->hm_hash(key);
    hash ^= (unsigned int) hash >> 16;

    struct rb_node *new_node = new_rb_node(key, hash, val, val_t);
    if (new_node == NULL) {
        fprintf(stderr, "failed to malloc new rb_node\n");
        return -1;
    }

    struct map_entry *entry = map->hm_tab + (hash & (map->hm_cap - 1));
    struct rb_node *node = entry->rbtree;

    /* 下面分两种情况，
     * entry 为红黑树根节点时，放到红黑树中
     * 否则，添加到链表中
     * *注意* 执行完之后，entry 是旧节点
     */
    if (_IS_RBTREE(node)) {
        node = put_rbtree(&(entry->rbtree), new_node, map->hm_cmp);
    }
    else {
        struct rb_node *last = NULL;
        while (node && (hash != node->hash || map->hm_cmp(key, node->key))) {
            last = node;
            node = node->part;
        }
        // 插入新元素或更新新元素
        if (node != NULL) 
            new_node->part = node->part;
        if (last != NULL)
            last->part = new_node;
        else 
            entry->rbtree = new_node;
    }

    if (node != NULL) {
        free(node);
        return 1;
    }

    map->hm_size ++;
    entry->size ++;
    if (! _IS_RBTREE(entry->rbtree) && entry->size >= map->tree_t) {
            to_rbtree(&(entry->rbtree), map->hm_cmp);
    }

    // 如果需要，对 hashmap 扩容
    if (resize_hashmap(map) == -1) {
        fprintf(stderr, "failed to resize_hashmap to %u capacity\n", 
            map->hm_cap << 1);
    }

    return 0;
}


static int resize_hashmap(struct hash_map *map)
{
    /**
      * 未达到负载因子，或者 hashmap 已经达到最大时，不进行扩容
      */
    if (map->hm_size <= (int) (map->hm_cap * map->hm_load) || 
        map->hm_size >= HASHMAP_MAX_CAPACITY) {
        return 0;
    }

    const int old_cap = map->hm_cap;
    const int new_cap = old_cap << 1;
    struct map_entry* new_tab = _REALLOC(struct map_entry, map->hm_tab, new_cap);

    if (new_tab == NULL) {
        return -1;
    }

    map->hm_cap = new_cap;
    map->hm_tab = new_tab;

    /* 接下来遍历每一个节点，进行再散列 */

    for (int i = 0; i < old_cap; i++) {
        struct map_entry *lo_entry = new_tab + i;
        struct rb_node *node = lo_entry->rbtree;

        if (_IS_RBTREE(node)) {
            un_rbtree(&node);
        }

        /* 根据 “旧 index 和新 index 是否相同”
         * 把链表拆分成 2 条新链表，新 index 为 i 和 i + old_cap
         */
        struct rb_node *lo_head = NULL, *lo_tail = NULL;
        struct rb_node *hi_head = NULL, *hi_tail = NULL;
        int lo_count = 0, hi_count = 0;
        struct rb_node *next;

        while (node) {
            next = node->part;
            node->part = NULL;

            if (node->hash & old_cap) {
                if (hi_head == NULL)
                    hi_head = node;
                else
                    hi_tail->part = node;
                hi_tail = node;
                hi_count ++;
            }
            else {
                if (lo_head == NULL)
                    lo_head = node;
                else
                    lo_tail->part = node;
                lo_tail = node;
                lo_count ++;
            }
            node = next;
        }
        struct map_entry *hi_entry = new_tab + i + old_cap;

        lo_entry->rbtree = lo_head;
        lo_entry->size = lo_count;
        hi_entry->rbtree = hi_head;
        hi_entry->size = hi_count;

        // 如果长度过长，转为红黑树
        if (lo_count >= map->tree_t) 
            to_rbtree(&(lo_entry->rbtree), map->hm_cmp);
        if (hi_count >= map->tree_t) 
            to_rbtree(&(hi_entry->rbtree), map->hm_cmp);
    }
    return 0;
}

int remove_hashmap(struct hash_map *map, const void *key)
{
    if (map == NULL) {
        return -1;
    }
    
    int hash = map->hm_hash(key);
    hash ^= (unsigned int) hash >> 16;

    struct map_entry *entry = map->hm_tab + (hash & (map->hm_cap - 1));
    struct rb_node *node = entry->rbtree;

    /** 接下来分两种情况，一种
      * 如果是红黑树，从红黑树中移除
      * 否则从链表中移除
      * *注意* 操作完后，node 为旧节点
      */

    if (_IS_RBTREE(node)) {
        node = remove_rbtree2(&(entry->rbtree), key, hash, map->hm_cmp);
    }
    else {
        struct rb_node *last = NULL;
        while (node && (hash != node->hash || map->hm_cmp(key, node->key))) {
            last = node;
            node = node->part;
        }
        // 更新链表
        if (node) {
            if (last)
                last->part = node->part;
            else 
                entry->rbtree = node->part;
        }
    }

    if (node == NULL) {
        return 0;
    }

    free(node);

    map->hm_size --;
    entry->size --;
    if (! _IS_RBTREE(entry->rbtree) && entry->size <= HASHMAP_DEF_UNTREE_THRESHOLD) {
        un_rbtree(&(entry->rbtree));
    }
    return 1;
}


static void un_rbtree(struct rb_node **root)
{
    struct rb_node *p, *node, *tail;
    node = tail = *root;

    while (node) {
        if ((p = node->left) != NULL) {
            node->left = NULL;
            p->part = NULL;
            tail->part = p;
            tail = p;
        }
        if ((p = node->right) != NULL) {
            node->right = NULL;
            p->part = NULL;
            tail->part = p;
            tail = p;
        }
        node->color = RB_RED;
        node = node->part;
    }
}

static void to_rbtree(struct rb_node **root, int (*cmp)(const void*, const void*))
{
    struct rb_node *node, *next = *root;
    while ((node = next) != NULL) {
        next = node->part;
        node->part = NULL;

        put_rbtree(root, node, cmp);
    }
}

int get_hashmap_size(struct hash_map *map)
{
    return map ? map->hm_size : 0;
}


int read_hashmap(struct hash_map *map, struct map_iterator *iter)
{
    if (map == NULL || iter == NULL) {
        return -1;
    }

    


    return 0;
}

#undef _MALLOC
#undef _REALLOC
#undef _IS_RBTREE