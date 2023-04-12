// Copyright Tudor Cristian-Andrei 311CAa 2022-2023
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
// free_func este functia specifica listei pentru a da free la data din nod
typedef struct {
	node_t *head;
	node_t *tail;
	size_t size;
	uint64_t data_size;
	void (*free_func)(void *data);
} list_t;

// parametrul alloc_fail l-am adaugat pentru a tine cont de fail-urile de
// alocare doar din cadrul arenei, iar parametrul minis_no pentru a retine
// numarul total de miniblock-uri din arena
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
	list_t *miniblock_list;
} block_t;

typedef struct {
	uint64_t start_address;
	size_t size;
	uint8_t perm;
	void *rw_buffer;
} miniblock_t;

// consider ca fiecare miniblock are o adresa, caracterizata de index-ul
// block-ului din arena, si index-ul miniblock-ului din block
// retin si nodurile in sine, pentru a nu le mai cauta inca o data
typedef struct {
	node_t *block;
	uint64_t b_idx;
	node_t *miniblock;
	uint64_t m_idx;
} address_t;

// o structura pentru permisiuni (este necesara doar pentru functie, ca sa
// returnez 3 valori deodata)
typedef struct {
	int8_t read;
	int8_t write;
	int8_t execute;
} permission_t;
