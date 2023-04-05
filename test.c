#include "DLL.h"
#include "structures.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define NMAX 2
void print_int(list_t *list) {
    node_t *curr = list->head;
    for (uint64_t i = 0; i < list->size; i++) {
        printf("%d -> ", *((int *)curr->data));
        curr = curr->next;
    }
    printf("|\n");
}

address_t *like_backup(arena_t *arena, uint64_t addr)
{
    // "adresa" este o pereche de coordonate
    address_t *pair = (address_t *)malloc(sizeof(address_t));
    if (pair == NULL) {
        arena->alloc_fail = -1;
        return NULL;
    }

    node_t *curr = arena->alloc_list->head;
    block_t *curr_block = (block_t *)curr->data;
    uint64_t block_idx = 0, miniblock_idx = 0;

    if (curr_block->start_address > addr) {
        free(pair);
        return NULL;
    }

    while (curr->next != NULL) {
        curr_block = (block_t *)curr->data;
        
        uint64_t start = curr_block->start_address;
        uint64_t end = start + curr_block->size;
        
        // testez daca adresa s - ar putea gasi in blocul curent
        if (start <= addr && addr <= end) {
            pair->block = curr;
            pair->b_idx = block_idx;

            node_t *mini = curr_block->miniblock_list->head;
            miniblock_idx = 0;
            while (mini != NULL) {
                if (addr == ((miniblock_t *)mini->data)->start_address) {
                    pair->miniblock = mini;
                    pair->m_idx = miniblock_idx;
                    return pair;
                }

                mini = mini->next;
                miniblock_idx++;
            }
            
            free(pair);
            return NULL;
        }

        curr = curr->next;
        block_idx++;
    }

    free(pair);
    return NULL;
}

int main()
{
    list_t *list1 = create(sizeof(int), free_int);
    list_t *list2 = create(sizeof(int), free_int);
    int aux;
    int8_t falsein = 0;
    for (int i = 0; i < NMAX; i++) {
        scanf("%d", &aux);
        add_nthNode(list1, i, &aux, &falsein);
    }

    print_int(list1);

    for (int i = 0; i < NMAX; i++) {
        scanf("%d", &aux);
        add_nthNode(list2, i, &aux, &falsein);
    }

    print_int(list2);

    mergeLists(list1, list2);
    print_int(list1);

    list_t *list3 = create(sizeof(int), free_int);

     for (int i = 0; i < NMAX; i++) {
        scanf("%d", &aux);
        add_nthNode(list3, i, &aux, &falsein);
    }

    print_int(list3);
    mergeLists(list3, list1);

    print_int(list3);
    free_list(&list3);
}
