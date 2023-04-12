// Copyright Tudor Cristian-Andrei 311CAa 2022-2023
#pragma once
#include "structures.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

list_t *create(uint64_t data_size, void (*free_func)(void *));
node_t *get_nthnode(list_t *list, uint64_t n);
void add_nthnode(list_t *list, uint64_t n, const void *data, int8_t *fail);
node_t *remove_firstnode(list_t *list);
node_t *remove_lastnode(list_t *list);
node_t *remove_nthnode(list_t *list, uint64_t n);
void free_list(list_t **list);
void free_blockdata(void *data);
void free_miniblockdata(void *data);
void mergelists(list_t *head_list, list_t *tail_list);
