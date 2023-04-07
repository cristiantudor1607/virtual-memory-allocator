#pragma once
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "structures.h"
#include "DLL.h"

#define LEFT_ADJACENT 10
#define BOTH_ADJACENT 20
#define RIGHT_ADJACENT 30

arena_t* alloc_arena(const uint64_t size);
void dealloc_arena(arena_t* arena);

void alloc_block(arena_t* arena, const uint64_t address, const uint64_t size);
void free_block(arena_t* arena, const uint64_t address);

void read(arena_t* arena, uint64_t address, uint64_t size);
void write(arena_t* arena, const uint64_t address,  const uint64_t size, int8_t *data);
void pmap(const arena_t* arena);
void mprotect(arena_t* arena, uint64_t address, int8_t *permission);

int find_place(arena_t *arena, uint64_t addr, size_t size);
block_t *get_block(arena_t *arena, uint64_t addr);
size_t get_position(list_t *list, uint64_t addr);
address_t *free_address(arena_t *arena, uint64_t addr);

miniblock_t *init_miniblock(uint64_t address, size_t size, uint8_t perm);
void add_newBlock(arena_t *arena, const uint64_t address, const uint64_t size);
void MergeAtLeft(arena_t *arena, const uint64_t addr, const uint64_t size);
void MergeAtRight(arena_t *arena, const uint64_t address, const uint64_t size);
void MergeBothSides(arena_t *arena, const uint64_t addr, const size_t size);

void delete_first(arena_t *arena, address_t *pair);
void delete_last(arena_t *arena, address_t *pair);
void delete_inside(arena_t *arena, address_t *pair);
size_t get_size(list_t *miniblock_list);
address_t *read_write_address(arena_t *arena, uint64_t addr);
void write_data(arena_t *arena, address_t *pair, uint64_t addr, int8_t *data, size_t size);
void read_data(arena_t *arena, address_t *pair, uint64_t addr, size_t size);

