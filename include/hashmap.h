

#ifndef _UTIL_HASHMAP_H
#define _UTIL_HASHMAP_H 1

#include "map.h"


/** 
  * hashmap 以 hash 作为键，可以实现非常快速的增改删查操作
  * 因此 key 的 hash 分布得越均匀，效率越高
  * 在 hash 的碰撞频率不大时，hashmap 保证了 O(1) 的时间复杂度
  * 如果发生了频繁碰撞，hashmap 会把链表转换为红黑树，
  * 提供 O(logn) 的时间复杂度，而不是传统的 O(n)
  * 
  * hashmap 不保证键值对的存储顺序
  *
  * 对于示例代码，请参考 example/ 
  * 
  */


/** 
  * hashmap 的最大容量
  * 超过这个值不会再自动扩容
  * *注意* 这个值指的是节点的个数，而不是占用空间
  * 必须是 2 的整次幂
  */
#define HASHMAP_MAX_CAPACITY    4096

/** 
  * hashmap 的默认负载因子
  * 即触发自动扩容的阈值
  * 当 size / capacity 达到这个阈值时，
  * 如果允许，将自动扩容为原来的 2 倍
  * *注意*，负载因子过大，会造成 hash 碰撞的几率增加；
  * 反之会浪费内存。因此这是个权衡利弊后的值
  * 如果没有特殊要求，不要尝试修改它
  */
#define HASHMAP_DEF_LOAD_FACTOR 0.75f

/** 
  * hashmap 的默认容量
  * 必须是 2 的整次幂
  */
#define HASHMAP_DEF_CAPACITY    16

/** 
  * 链表转化为红黑树的默认阈值
  * *注意* 这个值不允许比 4 小
  * 如果过大，将导致时间复杂度成为 O(n)
  */
#define HASHMAP_DEF_TREE_THRESHOLD      8

/** 
  * 红黑树转化为链表时的默认阈值
  * *注意* 这个值必须比 4 大，比 HASHMAP_DEF_TREE_THRESHOLD 小
  */
#define HASHMAP_DEF_UNTREE_THRESHOLD    6

/** 
  * 默认的 hashcode 函数实现
  * 它实际上使用地址作为 hashcode，
  * 因此不建议使用它
  */
#define HASHMAP_DEF_HASHCODE    _hm_ptr_hash


static int _hm_ptr_hash(const void *key)
{
    return *((int*) &key);
}


/** 默认的判断两个 key 是否相等的函数实现
  * 由于它只比较了地址，因此不建议使用
  */
#define HASHMAP_DEF_COMPARE     _hm_ptr_cmp


static int _hm_ptr_cmp(const void *one, const void *two)
{
    return (int) (one - two);
}


struct map_entry;


struct hash_map {
    /** hashmap 目前的容量
      * *注意* 这个值必须是 2 的整次幂
      */
    unsigned int hm_cap;

    /** hashmap 目前存储的节点数量
      * 它由系统自动维护
      */
    unsigned int hm_size;

    /** 链表转为红黑树的阈值
      * 参考 HASHMAP_DEF_TREE_THRESHOLD
      */
    unsigned int tree_t;

    /** 红黑树转为链表的阈值
      * 参考 HASHMAP_DEF_UNTREE_THRESHOLD
      */
    unsigned int untr_t;

    /** hashmap 自动扩容的负载因子
      * 当 size 和 capacity 的比值达到这个值时，
      * hashmap 会尝试扩容
      * 参考 HASHMAP_DEF_LOAD_FACTOR
      * *注意* 绝大多数情况下，使用默认值就好
      */
    float hm_load;

    /** 保存键值对的数组
      */
    struct map_entry *hm_tab;

    /** hashCode 函数，用来根据 key 计算出 hash
      * 大多数情况下，你需要更换它
      */
    int (*hm_hash) (const void*);

    /** 用来比较两个 key 是否相等的函数
      * 大多数情况下，你需要更换它
      */
    int (*hm_cmp) (const void*, const void*);
};

/**
  * 得到 hashmap 中已存储的键值对的数量
  * @param map hashmap
  * @return size
  */
int get_hashmap_size(struct hash_map *map);


/** 
  * 初始化 hashmap，并使用默认的配置
  *
  * @param dst 需要初始化的 hashmap 的指针
  * 如果为空，将会使用 malloc 动态分配，这种情况下请记得回收
  * @return 正常完成，返回 hashmap 的指针，出错返回 NULL
  */
struct hash_map* set_hashmap(struct hash_map *dst);


/** 
  * 将键值对保存到 hashmap
  *
  * @param map hashmap 的地址
  * @param key key 的地址
  * @param val value 的地址
  * @param val_t value 的长度，这个比较特殊
  * *注意* 如果 val_t 不是 0，那么 hashmap 会保存一份 value 的副本
  * 这个副本会拷贝 val 的内容，长度为 val_t 个字节
  *
  * @return 如果之前不存在 key，返回 0；否则返会 1
  * 出错返回 -1
  */
int put_hashmap(struct hash_map *map, const void *key, 
  const void *val, size_t val_t);


/** 
  * 根据 key 从 hashmap 中查找 value
  * 此函数会调用 map.hm_hash 计算 key 的 hash
  * 如果发生了 hash 碰撞，会调用 map.hm_cmp 比较 key 是否相等
  * 
  * @param map hashmap 的地址
  * @param key key 的地址
  * @return value 的地址，如果没找到，则返回 NULL
  * *注意* 如果在 put_hashmap() 中指定了 val_t，
  * 则会返回副本 value 的地址
  * 参考 put_hashmap
  */
void* get_hashmap(struct hash_map *map, const void *key);


/** 
  * 从 hashmap 中移除某一 key
  *
  * @param map hashmap 的地址
  * @param key key 的地址
  * @return 如果之前中不存在 key，返回 0；否则返回 1
  * 出错返回 -1
  */
int remove_hashmap(struct hash_map *map, const void *key);

/** 
  * 释放 hashmap 占用的所有内存(包括键值对)
  * 此后这个 hashmap 无法再次使用，
  * 除非使用 set_hashmap 重新分配内存
  * @param map 
  */
void free_hashmap(struct hash_map *map);

/** 
  * 清空并释放 hashmap 保存的所有 *键值对*
  * *注意* 此函数并没有彻底释放 hashmap 的内存
  * @param map
  */
void clear_hashmap(struct hash_map *map);


/** 
  * 得到 hashmap 的迭代器，用于遍历每一个键值对
  * @param map hashmap
  * @param iter iter
  * @return 完成返回 0，出错返回 -1
  */
int read_hashmap(struct hash_map *map, struct map_iterator *iter);


/** 
  * 对 hashmap 生成调试信息
  * @param map 
  */
void debug_hashmap(struct hash_map *map);

#endif /* _UTIL_HASHMAP_H_ */
