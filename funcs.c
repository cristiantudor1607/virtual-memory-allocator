#include "funcs.h"

void ALLOC_ARENA_func(arena_t **arena, char *command, int *memups)
{
    int check;
    check = check_validity(command, 2);
    if (check == 0)
        return;

    if (check == -1) {
        *memups = -1;
        //printf(MEMERR);
        return;
    }

    char *str_size = get_nth_arg(command, 2);
    if (str_size == NULL) {
        *memups = -1;
        //printf(MEMERR);
        return;
    }

    // strtoul returneaza 0 daca nu s - a putut face o conversie valida
    // sau daca dimensiunea data e 0
    uint64_t size = strtoul(str_size, NULL, 10);
    free(str_size);

    // pot sa pun conditia ca strcmp(strsize, 0 ) != 0 !!!
    if (size == 0) {
       // dau comanda de check la un nr mai mare de argumente 
       // pentru a afisa eroarea
       check_validity(command, BIG_NUM);
       return;
    }

    *arena = alloc_arena(size);
    if (*arena == NULL) {
        *memups = -1;
        //printf(MEMERR);
        return;
    }
}

void DEALLOC_ARENA_func(arena_t *arena, char *command, int *memups)
{
    int check;
    check = check_validity(command, 1);
    if (check == 0)
        return;

    if (check == -1) {
        *memups = -1;
        return;
    }

    dealloc_arena(arena);
    *memups = 0;
}

void ALLOC_BLOCK_func(arena_t **arena, char *command, int *memups)
{
    int check;
    check = check_validity(command, 3);
    if (check == 0)
        return;

    if (check == -1) {
        *memups = -1;
        return;
    }

    char *str_addr = get_nth_arg(command, 2);
    if (str_addr == NULL) {
        *memups = -1;
        return;
    }

    uint64_t addr = strtoul(str_addr, NULL, 10);

    if (addr == 0 && strcmp(str_addr, "0") != 0) {
        // dau check la un nr mare de argumente pentru a afisa eroarea
        check_validity(command, BIG_NUM);
        free(str_addr);
        return;
    }
    free(str_addr);

    char *str_size = get_nth_arg(command, 3);
    if (str_size == NULL) {
        *memups = -1;
        return;
    }

    size_t size = strtoul(str_size, NULL, 10);
    free(str_size);

    if (str_size == 0) {
        check_validity(command, BIG_NUM);
        return;
    }

    // acum am extras adrr si size - ul, urmeaza sa le pun in block
    if (addr > (*arena)->arena_size) {
        printf("The allocated address is outside the size of arena\n");
        return;
    }

    if (addr + size > (*arena)->arena_size) {
        printf("The end address is past the size of the arena\n");
        return;
    }

   alloc_block(*arena, addr, size);
   if ((*arena)->alloc_fail == -1) {
        *memups = -1;
        return;
   }

   (*arena)->minis_no++;
}

void FREE_BLOCK_func(arena_t *arena, char *command, int *memups)
{
    int check = check_validity(command, 2);
    if (check == 0)
        return;

    if (check == -1) {
        *memups = -1;
        return;
    }

    char *str_addr = get_nth_arg(command, 2);
    if (str_addr == NULL) {
        *memups = -1;
        return;
    }

    uint64_t addr = strtoul(str_addr, NULL, 10);
    
    if (addr == 0 && strcmp(str_addr, "0") != 0) {
        *memups = -1;
        free(str_addr);
        return;
    }
    
    free(str_addr);

    free_block(arena, addr);
    if (arena->alloc_fail == -1) {
        *memups = -1;
        return;
    }
}

void WRITE_func(arena_t *arena, char *command, int *memups)
{
    // aici nu mai merge validarea clasica
    char *addr_str = get_nth_arg(command, 2);
    if (addr_str == NULL) {
        *memups = -1;
        return;
    }

    // daca nu exista al doilea argument
    if (addr_str == command) {
        printf("Invalid command.Please try again.\n");
        return;
    }
    
    // salvez lungimea pentru a apela functia de get_bytes
    size_t addr_len = strlen(addr_str);

    uint64_t addr = strtoul(addr_str, NULL, 10);
    if (addr == 0 && strcmp(addr_str, "0") != 0) {
        // poate testezi failul la asta
        check_validity(command, 0);
        free(addr_str);
        return;
    }
    free(addr_str);

    char *size_str = get_nth_arg(command, 3);
    if (size_str == NULL) {
        *memups = -1;
        return;
    }

    // daca nu exista al treilea argument
    if (size_str == command) {
        printf("Invalid command.Please try again.\n");
        printf("Invalid command.Please try again.\n");
        return;
    }
    
    // salvez lungimea pt a apela functia de get_bytes
    size_t size_len = strlen(size_str);

    size_t size = strtoul(size_str, NULL, 10);
    if (size == 0) {
        // si la asta
        check_validity(command, 0);
        free(size_str);
        return;
    }
    free(size_str);

    char *bytes = get_bytes(command, addr_len, size_len);
    if (*bytes == '\0') {
        printf("Invalid command.Please try again.\n");
        printf("Invalid command.Please try again.\n");
        printf("Invalid command.Please try again.\n");
        return;
    }

    char *data = complete_arg(bytes, size);
    if (data == NULL) {
        *memups = -1;
        return;
    }
      
}

void READ_func(arena_t *arena, char *command, int *memups)
{
    int check = check_validity(command, 3);
    if (check == -1) {
        *memups = -1;
        return;
    }

    if (check == 0)
        return;

    char *str = get_nth_arg(command, 2);
    if (str == NULL) {
        *memups = -1; 
        return;
    }

    uint64_t addr = strtoul(str, NULL, 10);
    if (addr == 0 && strcmp(str, "0") != 0) {
        free(str);
        *memups = -1;
        return;
    }
    free(str);

    str = get_nth_arg(command, 3);
    if (str == NULL) {
        *memups = -1;
        return;
    }

    uint64_t size = strtoul(str, NULL, 10);
    free(str);
    if (size == 0) {
        *memups = -1;
        return;
    }

    printf("addr: %lu | size : %lu\n", addr, size);
}

void PMAP_func(arena_t *arena, char *command, int *memups)
{
    int check = check_validity(command, 1);
    if (check == 0)
        return;

    if (check == -1) {
        *memups = -1;
        return;
    }

    pmap(arena);
}
