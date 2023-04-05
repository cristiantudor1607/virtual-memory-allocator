#pragma once
#include "structures.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

list_t *create(uint64_t data_size, void (*free_func)(void *));
node_t *get_nthNode(list_t *list, uint64_t n);
void add_nthNode(list_t *list, uint64_t n , const void *data, int8_t *fail);
node_t *remove_firstNode(list_t *list);
node_t *remove_lastNode(list_t *list);
node_t *remove_nthNode(list_t *list, uint64_t n);
void free_list(list_t **list);
void free_blockData(void *data);
void free_miniblockData(void *data);
void free_int(void *data);
void mergeLists(list_t *head_list, list_t *tail_list);