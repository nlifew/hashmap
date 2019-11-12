

#ifndef _UTIL_LIST_H
#define _UTIL_LIST_H 1


struct list_node {
    void *value;
    struct list_node *prev;
    struct list_node *next;
};

struct list {
    unsigned int ls_size;
    struct list_node *ls_head;
    struct list_node *ls_tail;
};

struct list_node* add_list_head(struct list *list, void *val, size_t val_t);

struct list_node* add_list_tail(struct list *list, void *val, size_t val_t);

struct list_node* remove_list_head(struct list *list);

struct list_node* remove_list_tail(struct list *list);

void free_list(struct list *list);

#endif