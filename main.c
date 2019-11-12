

#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <string.h>

#include "include/hashmap.h"

int str_hash(const void *p)
{
    int hash = 0;
    
    for (char *str = (char*) p; *str != '\0'; str ++) {
        hash = 31 * hash + *str;
    }
    return hash;
}

int str_cmp(const void *p1, const void *p2)
{
    return strcmp((char*) p1, (char*) p2);
}

static char *keys[] = {
    "Hello",
    "what\'s that",
    "Hello",
    "is it works ?",
};

static char *vals[] = {
    "haha",
    "c",
    "world",
    "i feel good"
};

int main(int argc, char const *argv[])
{
    struct hash_map map;
    memset(&map, 0, sizeof(map));
    map.hm_cmp = str_cmp;
    map.hm_hash = str_hash;

    assert(set_hashmap(&map));

    for (int i = 0; i < 4; ++i) {
        char* key = keys [i];
        char *val = vals [i];
        put_hashmap(&map, key, val, strlen(val) + 1);
    }


    for (int i = 3; i > -1; --i) {
        char *p = (char*) get_hashmap(&map, keys[i]);
        printf("[%s, %s]\n", keys[i], p ? p : "null");
    }

    printf("------------------------------------------------------------->\n");


    remove_hashmap(&map, keys[1]);
    remove_hashmap(&map, keys[2]);

    for (int i = 3; i > -1; --i) {
        char *p = (char*) get_hashmap(&map, keys[i]);
        printf("[%s, %s]\n", keys[i], p ? p : "null");
    }

    free_hashmap(&map);

    return 0;
}