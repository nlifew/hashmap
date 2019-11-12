

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <memory.h>


#include "../include/hashmap.h"

int int_hash(const void *p)
{
    return *((int*) p);
}

int int_cmp(const void *p1, const void *p2)
{
    return *((int*) p1) - *((int*) p2);
} 


int main(int argc, char const *argv[])
{
    /* 开始初始化 hashmap */

    struct hash_map map;
    memset(&map, 0, sizeof(map));   //必须的
    map.hm_cmp = int_cmp;
    map.hm_hash = int_hash;     // 替换为我们自己的函数

    if (set_hashmap(&map) == NULL) {
        fprintf(stderr, "failed to init hashmap\n");
        return 1;
    }

    // 不保留 value 的副本，而是 value 这个值
    int key = 21, val = -21;
    put_hashmap(&map, &key, &val, 0);
    int *p = (int*) get_hashmap(&map, &key);
    printf("val[%d]\n", p ? *p : -1);

    // 更改 val 的值
    val = 22;
    p = (int*) get_hashmap(&map, &key);
    printf("val[%d]\n", p ? *p : -1);

    // 移除 key
    remove_hashmap(&map, &key);
    p = (int*) get_hashmap(&map, &key);
    printf("val[%d]\n", p ? *p : -1);

    // 这次保存 value 的副本
    put_hashmap(&map, &key, &val, sizeof(val));
    p = (int*) get_hashmap(&map, &key);
    printf("val[%d]\n", p ? *p : -1);

    // 更改 val 的值
    val = 55;
    p = (int*) get_hashmap(&map, &key);
    printf("val[%d]\n", p ? *p : -1);

    // 移除 key
    remove_hashmap(&map, &key);
    p = (int*) get_hashmap(&map, &key);
    printf("val[%d]\n", p ? *p : -1);
    
    // 回收资源
    free_hashmap(&map);

    return 0;
}
