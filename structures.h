#pragma once
#include <inttypes.h>
#include <stddef.h>
// structura pentru nod cu date generice
typedef struct node_t node_t;
struct node_t {
   void *data;
   node_t *next;
   node_t *prev;
};

// structura pentru lista
// free_func este functia specifica listei pentru free
typedef struct {
   node_t *head;
   node_t *tail;
   size_t size;
   uint64_t data_size;
   void (*free_func)(void *data);
} list_t;

// parametrul alloc_fail este pentru a tine cont de fail - urile de
// alocare dinamica din cadrul arenei
typedef struct {
   uint64_t arena_size;
   uint64_t free_memory;
   uint64_t minis_no;
   list_t *alloc_list;
   int8_t alloc_fail;
} arena_t;

typedef struct {
   uint64_t start_address; 
   size_t size; 
   list_t* miniblock_list;
} block_t;

typedef struct {
   uint64_t start_address;
   size_t size;
   uint8_t perm;
   void* rw_buffer;
} miniblock_t;

// consider ca fiecare miniblock are o adresa "reala", caracterizata prin
// indexul block-ului in care se afla, din arena, si index-ul din lista de
// minblock-uri din care face parte
// retin atat nodul, cat si index-ul pentru a le folosi mai departe
typedef struct {
   node_t *block;
   uint64_t b_idx;
   node_t *miniblock;
   uint64_t m_idx;
} address_t;

// o structura care sa imi returneze ce permisiuni are un miniblock
typedef struct {
   int8_t read;
   int8_t write;
   int8_t execute;
} permission_t;