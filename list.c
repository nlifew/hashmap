

#ifndef _UTIL_LIST_H_
#define _UTIL_LIST_H_ 1

#include <stdlib.h>
#include <memory.h>

#include "private/list.h"

static struct list_node* new_list_node(void *val, size_t val_t)
{
    const size_t mem_t = sizeof(struct list_node) + val_t;
    struct list_node *node = (struct list_node*) malloc(mem_t);

    if (node == NULL) {
        return NULL;
    }
    node->value = val;
    node->prev = node->next = NULL;

    if (val_t != 0) {
        node->value = node + 1;
        memcpy(node->value, val, val_t);
    }

    return node;
}

struct list_node* add_list_head(struct list *list, void *val, size_t val_t)
{
    struct list_node *node;

    if (list == NULL || (node = new_list_node(val, val_t)) == NULL) {
        return NULL;
    }

    if (list->ls_head == NULL) {    
        list->ls_tail = node;
    }
    else {
        list->ls_head->prev = node;
        node->next = list->ls_head;
    }
    list->ls_size ++;
    list->ls_head = node;
    return node;
}

struct list_node* add_list_tail(struct list *list, void *val, size_t val_t)
{
    struct list_node *node;

    if (list == NULL || (node = new_list_node(val, val_t)) == NULL) {
        return NULL;
    }

    if (list->ls_tail == NULL) {    
        list->ls_head = node;
    }
    else {
        list->ls_tail->next = node;
        node->prev = list->ls_tail;
    }
    list->ls_size ++;
    list->ls_tail = node;
    return node;
}

struct list_node* remove_list_head(struct list *list) 
{
    if (list == NULL || list->ls_head == NULL) {
        return NULL;
    }
    struct list_node *node = list->ls_head;
    if (node->next != NULL) {
        node->next->prev = NULL;
    }
    else {
        list->ls_tail = NULL;
    }

    list->ls_head = node->next;
    list->ls_size --;
    node->next = NULL;
    return node;
}

struct list_node* remove_list_tail(struct list *list) 
{
    if (list == NULL || list->ls_tail == NULL) {
        return NULL;
    }
    struct list_node *node = list->ls_tail;
    if (node->prev != NULL) {
        node->prev->next = NULL;
    }
    else {
        list->ls_head = NULL;
    }
    list->ls_tail = node->prev;
    node->prev = NULL;
    list->ls_size --;
    return node;
}

void free_list(struct list *list)
{
    if (list == NULL) 
        return;

    struct list_node *next, *node = list->ls_head;

    while (node != NULL) {
        next = node->next;
        free(node);
        node = next;
    }
    memset(list, 0, sizeof(struct list));
}

#endif