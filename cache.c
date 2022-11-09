#include "cache.h"

cache_list *cache_init(){
    cache_list *c_list = (cache_list *)Malloc(sizeof(cache_list));
    c_list->current_length = 0;
    c_list->front = NULL;
    c_list->rear = NULL;
    return c_list;
}

char* find_cache(cache_list* lst, char *url){
    if (lst->front == NULL){
        return NULL; //  if cache lst is empty
    }
    for(cache_node* i = lst->front; i != NULL; i = i->next){
        if (strcmp(i->payload, url) == 0){ // find in cache lst
            if (i == lst->rear){
                push(lst, pop(lst));  
            } else if (i != lst->front){
                delete(lst, i);
                push(lst, i);
            }
            return i->payload;
        }
    }
    return NULL; //cache miss.
}

void insert_cache(cache_list* lst, char *url, char* data){
    cache_node *new_node = (cache_node*)Malloc(sizeof(cache_node));
    strcpy(new_node->head, url);
    strcpy(new_node->payload, data);
    if (lst->current_length == MAX_CACHE_COUNT){
        Free(pop(lst)); // delete rear node and free.
    }
    push(lst, new_node);
}

void push(cache_list* list, cache_node* node){
    if (list->current_length == 0){
        list->front = node;
        node->prev = NULL;
        list->current_length++;
        return;
    }
    list->front->prev = node;
    node->next = list->front;
    node->prev = NULL;
    list->front = node;
    list->current_length++;
    return ;
}

cache_node* pop(cache_list *list) {
    cache_node* targetnode = list->rear;
    targetnode->prev->next = NULL;
    list->rear = targetnode->prev;
    list->current_length--;
    return targetnode;
}

void delete(cache_list *lst, cache_node* node){
    node->prev->next = node->next;
    node->next->prev = node->prev;
    lst->current_length--;
}

void del_cache(cache_list *list){
    while (list->current_length > 0){
        Free(pop(list));
    }
    Free(list);
}