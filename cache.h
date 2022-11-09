

#ifndef _CACHE_H_
#define _CACHE_H_

#include "csapp.h"
#define MAX_OBJECT_SIZE 102400
#define MAX_LINE 8196
#define MAX_CACHE_COUNT 10

typedef struct cache_node{
    char head[MAX_LINE];
    char payload[MAX_OBJECT_SIZE];
    struct cache_node* prev;
    struct cache_node* next;
} cache_node;

typedef struct cache_list{
    int current_length;
    cache_node* front;
    cache_node* rear;
}cache_list;

cache_list* cache_init(); // init cache_list.
void insert_cache(cache_list* lst, char* url, char* data); // insert new node to cache_list.
char* find_cache(cache_list *lst, char* url); // find in cache list.
void del_cache(); // free all cache node and list.
cache_node *pop(cache_list* lst); // rear node pop
void push(cache_list* lst, cache_node* node); //  node push at front of lst
void delete(cache_list* lst, cache_node* node); // middle node delete

#endif