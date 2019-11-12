

#ifndef _UTIL_MAP_H
#define _UTIL_MAP_H 1

struct map_iterator
{
    void *key;
    void *value;
    int offset;
    void *tag;
    int (*has_next)(struct map_iterator* iter, void* map);
};

#endif