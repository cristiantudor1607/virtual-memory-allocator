// Copyright Tudor Cristian-Andrei 311CAa 2022-2023
#pragma once
#include "vma.h"
#include "text_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MEMERR "Memory allocation error: not enough memory available.\n"
#define BIG_NUM 10000

void ALLOC_ARENA_func(arena_t **arena, char *command, int *memups);
void DEALLOC_ARENA_func(arena_t *arena, char *command, int *memups);
void ALLOC_BLOCK_func(arena_t **arena, char *command, int *memups);
void FREE_BLOCK_func(arena_t *arena, char *command, int *memups);
void PMAP_func(arena_t *arena, char *command, int *memups);
void WRITE_func(arena_t *arena, char *command, int *memups);
void READ_func(arena_t *arena, char *command, int *memups);
void MPROTECT_func(arena_t *arena, char *command, int *memups);
