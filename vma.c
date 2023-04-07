#include "vma.h"

// functie care cauta prin lista de blocuri, daca noul bloc cu parametrii
// addr si size are loc in arena
// functia intoarce 0 daca nu are loc, 1 daca are loc si nu are alte blocuri
// adiacente, sau LEFT_ADJACENT, RIGHT_ADJACENT, sau BOTH_ADJACENT, daca are
// blocuri adiacente
int find_place(arena_t *arena, uint64_t addr, size_t size)
{
    node_t *curr_node = arena->alloc_list->head;
    node_t *next_node = NULL;
    uint64_t curr_addr, next_addr;
    size_t curr_size, next_size;

    if (curr_node == NULL)
        return 1;

    // testez daca ar avea loc la inceputul arenei
    curr_addr = ((block_t *)curr_node->data)->start_address;

    if (addr + size < curr_addr)
        return 1;

    if (addr + size == curr_addr)
        return RIGHT_ADJACENT;
    
    while (curr_node->next != NULL) {
        curr_addr = ((block_t *)curr_node->data)->start_address;
        curr_size = ((block_t *)curr_node->data)->size;

        next_node = curr_node->next;
        next_addr = ((block_t *)next_node->data)->start_address;
        next_size = ((block_t *)next_node->data)->size;

        if ((curr_addr + curr_size < addr) && (addr + size < next_addr))
            return 1;

        if ((curr_addr + curr_size == addr) && (addr + size < next_addr))
            return LEFT_ADJACENT;

        if ((curr_addr + curr_size < addr) && (addr + size == next_addr))
            return RIGHT_ADJACENT;

        if ((curr_addr + curr_size == addr) && (addr + size == next_addr))
            return BOTH_ADJACENT;
        
        curr_node = curr_node->next;
    }

    // testez daca ar avea loc la finalul listei

    curr_addr = ((block_t *)curr_node->data)->start_address;
    curr_size = ((block_t *)curr_node->data)->size;
    if (curr_addr + curr_size < addr)
        return 1;

    if (curr_addr + curr_size == addr)
        return LEFT_ADJACENT;

    return 0; 
}

size_t get_position(list_t *list, uint64_t addr)
{
    node_t *curr = list->head;
    size_t idx = 0;
    uint64_t curr_addr = 0, next_addr = 0;  

    if (list->head == NULL)
        return 0;
    
    // testez daca trebuie adaugata la inceputul listei
    // la momentul apelarii functiei se stie deja ca are loc in lista
    if (list->data_size == sizeof(block_t))
        curr_addr = ((block_t *)curr->data)->start_address;
    else if (list->data_size == sizeof(miniblock_t))
        curr_addr = ((miniblock_t *)curr->data)->start_address;

    if (curr_addr > addr)
        return 0;

    while (curr->next != NULL) {
        idx++;
        if (list->data_size == sizeof(block_t)) {
            curr_addr = ((block_t *)curr->data)->start_address;
            next_addr = ((block_t *)curr->next->data)->start_address;
        } else if (list->data_size == sizeof(miniblock_t)) {
            curr_addr = ((miniblock_t *)curr->data)->start_address;
            next_addr = ((miniblock_t *)curr->next->data)->start_address;
        }

        if ((curr_addr < addr) && (addr < next_addr))
            return idx;

        curr = curr->next;
    }

    idx++;

    // testez daca are loc pe ultima pozitie
    if (list->data_size == sizeof(block_t))
        curr_addr = ((block_t *)curr->data)->start_address;
    else if (list->data_size == sizeof(miniblock_t))
        curr_addr = ((miniblock_t *)curr->data)->start_address;

    if (curr_addr < addr)
        return idx;

    // nu se ajunge niciodata sa nu se dea return la ceva pentru ca
    // verificarile au fost facute inainte, dar trebuie sa dau return
    // la ceva pt warninguri
    return 0;
}

miniblock_t *init_miniblock(uint64_t address, size_t size, uint8_t perm)
{
    miniblock_t *new_mini = malloc(sizeof(miniblock_t));
    if (new_mini == NULL)
        return NULL;

    new_mini->start_address = address;
    new_mini->size = size;
    new_mini->perm = perm;
    new_mini->rw_buffer = NULL;
    return new_mini;
}

void add_newBlock(arena_t *arena, const uint64_t address, const uint64_t size)
{
    // creez un nou block ce trebuie adaugat in arena
    block_t *new_block = malloc(sizeof(block_t));
    if (new_block == NULL) {
        arena->alloc_fail = -1;
        return;
    }
    new_block->size = size;
    new_block->start_address = address;

    // creez lista de miniblock -uri aferenta block - ului
    new_block->miniblock_list = create(sizeof(miniblock_t), free_miniblockData);
    if (new_block->miniblock_list == NULL) {
        arena->alloc_fail = -1;
        free(new_block);
        return;
    }

    // creez primul miniblock din lista de miniblock - uri, adica
    // block ul pe care il aloc propriu-zis

    miniblock_t *new_miniblock = init_miniblock(address, size, 6);
    if (new_miniblock == NULL) {
        arena->alloc_fail = -1;
        // se schimba free - ul din free list
        free_list(&new_block->miniblock_list);
        free(new_block);
        return;
    }

    int8_t memups = 0;
    add_nthNode(new_block->miniblock_list, 0, new_miniblock, &memups);
    free(new_miniblock);
    if (memups == -1) {
        arena->alloc_fail = -1;
        free_list(&new_block->miniblock_list);
        free(new_block);
        return;
    }

    size_t idx = get_position(arena->alloc_list, address);
    add_nthNode(arena->alloc_list, idx, new_block, &memups);
    free(new_block);
    if (memups == -1) {
        arena->alloc_fail = -1;
        free(new_miniblock);
        free_list(&new_block->miniblock_list);
        return;
    }

    // scad din memoria libera a arenei
    arena->free_memory = arena->free_memory - size;
}

void MergeAtRight(arena_t *arena, const uint64_t address, const uint64_t size)
{
    miniblock_t *new_mini = init_miniblock(address, size, 6);
    if (new_mini == NULL) {
        arena->alloc_fail = -1;
        return;
    }

    // nu o sa returneze niciodata 0, deoarce nu poti sa faci merge la dreapta
    // pe pozitia 0
    size_t block_idx = get_position(arena->alloc_list, address) - 1;
    node_t *block = get_nthNode(arena->alloc_list, block_idx);
    
    // modific doar dimensiunea blocului, deoarece daca adaug la dreapta, o
    // sa fie adaugat la final
    int8_t fail = 0;
    uint64_t idx = ((block_t *)block->data)->miniblock_list->size;
    add_nthNode(((block_t *)block->data)->miniblock_list, idx, new_mini, &fail);
    free(new_mini);
    if (fail == -1) {
        arena->alloc_fail = -1;
        return;
    }
    ((block_t *)block->data)->size += size;
    arena->free_memory = arena->free_memory - size;

}

void MergeAtLeft(arena_t *arena, const uint64_t addr, const uint64_t size) {
    miniblock_t *new_mini = init_miniblock(addr, size, 6);
    if (new_mini == NULL) {
        arena->alloc_fail = -1;
        return;
    }

    // nu o sa intoarca niciodata ultima pozitie deoarece nu are adiacent la dreapta
    // pe ultima pozitie
    uint64_t block_idx = get_position(arena->alloc_list, addr);
    node_t *block = get_nthNode(arena->alloc_list, block_idx);
    
    // daca ii dau merge left o sa fie adaugat pe prima pozitie
    int8_t fail = 0;
    add_nthNode(((block_t *)block->data)->miniblock_list, 0, new_mini, &fail);
    free(new_mini);
    if (fail == -1) {
        arena->alloc_fail = -1;
        return;
    }

    ((block_t *)block->data)->size += size;
    ((block_t *)block->data)->start_address = addr;
    arena->free_memory = arena->free_memory - size;
}

void MergeBothSides(arena_t *arena, const uint64_t addr, const size_t size)
{
    // as putea sa folosesc oricare dintre functiile de merge,
    // dar ca sa nu caut de 2 ori acelasi nod prin lista, o sa fac de la
    // inceput

    miniblock_t *new_mini = init_miniblock(addr, size, 6);
    if (new_mini == NULL) {
        arena->alloc_fail = -1;
        return;
    }

    uint64_t block_idx = get_position(arena->alloc_list, addr);
    node_t *block = get_nthNode(arena->alloc_list, block_idx);
    node_t *previous = block->prev;
    int8_t fail = 0;

    // adaug pe prima pozitie in bloc
    add_nthNode(((block_t *)block->data)->miniblock_list, 0, new_mini, &fail);
    free(new_mini);
    if (fail == -1) {
        arena->alloc_fail = -1;
        return;
    }

    ((block_t *)block->data)->size += size;
    ((block_t *)block->data)->start_address = addr;
    arena->free_memory = arena->free_memory - size;

    ((block_t *)previous->data)->size += ((block_t *)block->data)->size;
    mergeLists(((block_t *)previous->data)->miniblock_list, 
                ((block_t *)block->data)->miniblock_list);
    block = remove_nthNode(arena->alloc_list, block_idx);
    free((block_t *)block->data);
    free(block);

}

// de terminat functia asta
address_t *free_address(arena_t *arena, uint64_t addr)
{
    address_t *pair = malloc(sizeof(address_t));
    if (pair == NULL) {
        arena->alloc_fail = -1;
        return NULL;
    }

    uint64_t block_idx = 0, mini_idx = 0;

    node_t *curr = arena->alloc_list->head;

    // daca nu exista niciun block returnez NULL
    if (curr == NULL) {
        free(pair);
        return NULL;
    }

    while (curr != NULL) {
        uint64_t start = ((block_t *)curr->data)->start_address;
        uint64_t end = start + ((block_t *)curr->data)->size;

        if (start <= addr && addr <= end) {
            pair->block = curr;
            pair->b_idx = block_idx;

            node_t *aux = ((block_t *)curr->data)->miniblock_list->head;
            mini_idx = 0;
            while (aux != NULL) {
                if (addr == ((miniblock_t *)aux->data)->start_address) {
                    pair->miniblock = aux;
                    pair->m_idx = mini_idx;
                    return pair;
                }

                aux = aux->next;
                mini_idx++;
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

void delete_first(arena_t *arena, address_t *pair)
{
    node_t *block = pair->block;
    node_t *miniblock = pair->miniblock;
    // pastrez datele miniblock-ului pentru a updata block - ul
    uint64_t addr = ((miniblock_t *)miniblock->data)->start_address;
    size_t size = ((miniblock_t *)miniblock->data)->size;
    
    node_t *remove = remove_nthNode(((block_t *)block->data)->miniblock_list, 0);
    free(((miniblock_t *)remove->data)->rw_buffer);
    free(remove->data);
    free(remove);

    arena->free_memory += size;
    //arena->arena_size -= size;
    arena->minis_no--;
    ((block_t *)block->data)->start_address += size;
    ((block_t *)block->data)->size -= size;

    // daca e unicul miniblock trebuie sters

    if (((block_t *)block->data)->size == 0) {
        remove = remove_nthNode(arena->alloc_list, pair->b_idx);
        // intai eliberez lista de miniblocuri
        list_t *list = ((block_t *)remove->data)->miniblock_list;
        free_list(&list);
        free(remove->data);
        free(remove);
    }
}

void delete_last(arena_t *arena, address_t *pair)
{
    node_t *block = pair->block;
    node_t *miniblock = pair->miniblock;

    // pastrez datele miniblock-ului pentru a updata block - ul
    uint64_t addr = ((miniblock_t *)miniblock->data)->start_address;
    size_t size = ((miniblock_t *)miniblock->data)->size;

    uint64_t last = ((block_t *)block->data)->miniblock_list->size - 1;

    node_t *remove = remove_nthNode(((block_t *)block->data)->miniblock_list, last);
    free(((miniblock_t *)remove->data)->rw_buffer);
    free(remove->data);
    free(remove);

    arena->free_memory += size;
    arena->minis_no--;
    
    // adresa de start nu se modifica, doar cea de end, dar e calculata
    // cu ajutorul lui size
    ((block_t *)block->data)->size -= size;

    // daca era unicul miniblock sterg blocul

    if (((block_t *)block->data)->size == 0) {
        remove = remove_nthNode(arena->alloc_list, pair->b_idx);
        list_t *list = ((block_t *)remove->data)->miniblock_list;
        free_list(&list);
        free(remove->data);
        free(remove);
    }
}

size_t get_size(list_t *miniblock_list)
{
    node_t *curr = miniblock_list->head;
    size_t size  = 0;
    while (curr != NULL) {
        size += ((miniblock_t *)curr->data)->size;
        curr = curr->next;
    }

    return size;
}

void delete_inside(arena_t *arena, address_t *pair)
{
    node_t *block = pair->block;
    node_t *miniblock = pair->miniblock;

    // pastrez datele miniblock - ului

    uint64_t addr = ((miniblock_t *)miniblock->data)->start_address;
    size_t size = ((miniblock_t *)miniblock->data)->size;

    // am nevoie sa creez un nou block, si lista din el

    block_t *new_block = malloc(sizeof(block_t));
    if (new_block == NULL) {
        arena->alloc_fail = -1;
        return;
    }

    new_block->miniblock_list = create(sizeof(miniblock_t), free_miniblockData);
    if (new_block->miniblock_list == NULL) {
        arena->alloc_fail = -1;
        free(new_block);
        return;
    }
    list_t *list = ((block_t *)block->data)->miniblock_list;
    // initializez lista
    node_t *new_head = miniblock->next;
    node_t *new_tail = miniblock->prev;

    new_block->miniblock_list->head = new_head;
    new_block->miniblock_list->tail = list->tail;

    size_t init_size = list->size;
    new_block->miniblock_list->size = init_size - pair->m_idx - 1;


    node_t *remove = remove_nthNode(((block_t *)block->data)->miniblock_list, pair->m_idx);
    free(((miniblock_t *)remove->data)->rw_buffer);
    free(remove->data);
    free(remove);

    // acum new_tail este legat la new_head
    
    new_tail->next = NULL;
    list->tail = new_tail;
    list->size = pair->m_idx;
    new_head->prev = NULL;

    new_block->start_address = addr + size;
    new_block->size = get_size(new_block->miniblock_list);

    // mai trebuie facute schimbari si in blocul original
    ((block_t *)block->data)->size -= (new_block->size + size);
    arena->free_memory += size;
    arena->minis_no--;
    int8_t memups = 0;
    add_nthNode(arena->alloc_list, pair->b_idx + 1, new_block, &memups);
    if (memups == -1) {
        free_list(&new_block->miniblock_list);
        free(new_block);
        arena->alloc_fail = -1;
        return;
    }

    free(new_block);
}

address_t *read_write_addr(arena_t *arena, uint64_t addr)
{
    address_t *pair = malloc(sizeof(address_t));
    if (pair == NULL) {
        arena->alloc_fail = -1;
        return NULL;
    }

    node_t *curr_block = arena->alloc_list->head;
    uint64_t block_idx = 0;
    while (curr_block != NULL) {
        uint64_t start = ((block_t *)curr_block->data)->start_address;
        uint64_t end = start + ((block_t *)curr_block->data)->size;
        if (start <= addr && addr < end) {
            pair->b_idx = block_idx;
            pair->block = curr_block;

            list_t *list = ((block_t *)curr_block->data)->miniblock_list;
            node_t *curr_mini = list->head;
            uint64_t miniblock_idx = 0;
            while (curr_mini != NULL) {
                start = ((miniblock_t *)curr_mini->data)->start_address;
                end = start + ((miniblock_t *)curr_mini->data)->size;
                if (start <= addr && addr < end) {
                    pair->m_idx = miniblock_idx;
                    pair->miniblock = curr_mini;
                    return pair;
                }

                miniblock_idx++;
                curr_mini = curr_mini->next;
            }
        }

        block_idx++;
        curr_block = curr_block->next;
    }
    
    free(pair);
    return NULL;
}

void write_data(arena_t *arena, address_t *pair, uint64_t addr, int8_t *data, size_t size)
{
    size_t init_size = size;
    node_t *curr = pair->miniblock;
    miniblock_t *minblock = (miniblock_t *)curr->data;
    if (minblock->rw_buffer == NULL) {
        minblock->rw_buffer = (int8_t *)calloc(minblock->size, sizeof(int8_t));
        
        if (minblock->rw_buffer == NULL) {
            arena->alloc_fail = -1;
            return;
        }
    }

    uint64_t start_idx = addr - minblock->start_address;

    int8_t *buff_ptr = (int8_t *)minblock->rw_buffer + start_idx;
    int8_t *data_ptr = data;
    uint64_t end = minblock->start_address + minblock->size;
    do {
        *buff_ptr = *data_ptr;
        data_ptr++;
        addr++;
        size--; 
        if (addr == end) {
            curr = curr->next;
            if (curr == NULL)
                break;
            minblock = (miniblock_t *)curr->data;
            uint64_t buff_size = minblock->size;
            if (minblock->rw_buffer == NULL) {
                minblock->rw_buffer = (int8_t *)calloc(buff_size, sizeof(int8_t));
                if (minblock->rw_buffer == NULL) {
                    arena->alloc_fail = -1;
                    return;
                }
            }

            buff_ptr = (int8_t *)minblock->rw_buffer;
            end = minblock->start_address + buff_size;
            continue;
        }

        buff_ptr++;
    } while (size > 0 && curr != NULL);

    if (size > 0) {
        printf("Warning: size was bigger than the block size. ");
        printf("Writing %lu characters.\n", init_size - size);
    }
}

void read_data(arena_t *arena, address_t *pair, uint64_t addr, size_t size) {
    // calculez daca pot citi toate caracterele
    uint64_t block_start = ((block_t *)pair->block->data)->start_address;
    size_t block_size = ((block_t *)pair->block->data)->size;
    uint64_t block_end = block_start + block_size;
    if (addr + size > block_end) {
        printf("Warning: size was bigger than the block size. ");
        printf("Reading %lu characters.\n", block_end - addr);
    }
    
    node_t *curr = pair->miniblock;
    miniblock_t *minblock = (miniblock_t *)curr->data;
    
    uint64_t start_idx = addr - minblock->start_address;
    int8_t *buff_ptr = (int8_t *)minblock->rw_buffer + start_idx;

    uint64_t end = minblock->start_address + minblock->size;

    do {
        printf("%c", *buff_ptr);
        addr++;
        size--;
        if (addr == end) {
            curr = curr->next;
            if (curr == NULL)
                break;;

            minblock = (miniblock_t *)curr->data;
            buff_ptr = (int8_t *)minblock->rw_buffer;
            end = minblock->start_address + minblock->size;
            continue;
        }

        buff_ptr++;

    } while (size > 0 && curr != NULL);

    printf("\n");
}

arena_t *alloc_arena(const uint64_t size)
{
    arena_t *new_arena = (arena_t *)malloc(sizeof(arena_t));
    new_arena->arena_size = size;
    new_arena->free_memory = size;
    new_arena->alloc_fail = 1;
    new_arena->minis_no = 0;
    new_arena->alloc_list = create(sizeof(block_t), free_blockData);
    if (new_arena->alloc_list == NULL) {
        new_arena->alloc_fail = -1;
        return NULL;
    }

    return new_arena;
}

void dealloc_arena(arena_t *arena)
{
    free_list(&arena->alloc_list);
    free(arena);
}

void alloc_block(arena_t *arena, const uint64_t address, const uint64_t size)
{
    int put = find_place(arena, address, size);
    if (put == 0) {
        printf("This zone was already allocated.\n");
        return;
    }

    if (put == 1) {
        add_newBlock(arena, address, size);
        arena->minis_no++;
        return;
    }

    // daca are adiacent la stanga, trebuie sa dau merge la dreapta
    if (put == LEFT_ADJACENT) {
        MergeAtRight(arena, address, size);
        arena->minis_no++;
        return;
    }

    if (put == RIGHT_ADJACENT) {
        MergeAtLeft(arena, address, size);
        arena->minis_no++;
        return;
    }

    if (put == BOTH_ADJACENT) {
        MergeBothSides(arena, address, size);
        arena->minis_no++;
        return;
    }
    
}

void free_block(arena_t *arena, const uint64_t address)
{
    address_t *pair = free_address(arena, address);
    if (arena->alloc_fail == -1)
        return;

    if (pair == NULL) {
        printf("Invalid address for free.\n");
        return;
    }

    if (pair->m_idx == 0) {
        delete_first(arena, pair);
        free(pair);
        return;
    }

    size_t list_size = ((block_t *)pair->block->data)->miniblock_list->size;
    if (pair->m_idx == list_size - 1) {
        delete_last(arena, pair);
        free(pair);
        return;
    }

    delete_inside(arena, pair);
    free(pair);

}

void read(arena_t *arena, uint64_t address, uint64_t size)
{
    address_t *pair = read_write_addr(arena, address);

    if (arena->alloc_fail == -1)
        return;

    if (pair == NULL) {
        printf("Invalid address for read.\n");
        return;
    }

    read_data(arena, pair, address, size);
    free(pair);
}

void write(arena_t *arena, const uint64_t address, const uint64_t size, int8_t *data)
{
    // obtin blocul si miniblocul de unde incep sa scriu
    address_t *pair = read_write_addr(arena, address);
    if (arena->alloc_fail == -1)
        return;
    
    // cazul in care nu s-a gasit
    if (pair == NULL) {
        printf("Invalid address for write.\n");
        return;
    }

    write_data(arena, pair, address, data, size);
    free(pair);
}

// se schimba %lu cu %X
void pmap(const arena_t *arena)
{
    uint64_t mem_total = arena->arena_size;
    uint64_t mem_free = arena->free_memory;
    uint64_t minis_no = arena->minis_no;
    uint64_t blocks_no = arena->alloc_list->size;
    printf("Total memory: 0x%lX bytes\n", mem_total);
    printf("Free memory: 0x%lX bytes\n", mem_free);
    printf("Number of allocated blocks: %lu\n", blocks_no);
    printf("Number of allocated miniblocks: %lu\n", minis_no);

    node_t *curr_blck = arena->alloc_list->head;
    for (uint64_t i = 0; i < blocks_no; i++) {
        printf("\nBlock %lu begin\n", i + 1);

        uint64_t start = ((block_t *)curr_blck->data)->start_address;
        uint64_t end = start + ((block_t *)curr_blck->data)->size; 
        printf("Zone: 0x%lX - 0x%lX\n", start, end);

        node_t *curr_mini = ((block_t *)curr_blck->data)->miniblock_list->head;
        // pentru start si end de la miniblock-uri folosesc aceleasi variabile
        // pe care le - am folosit pentru block-uri
        
        // pentru numarul de nodurile din liste pot sa - l folosesc pe minis_no
        minis_no = ((block_t *)curr_blck->data)->miniblock_list->size;
        for (uint64_t j = 0; j < minis_no; j++) {
            start = ((miniblock_t *)curr_mini->data)->start_address;
            end = start + ((miniblock_t *)curr_mini->data)->size;
            printf("Miniblock %lu:", j + 1);
            printf("\t\t0x%lX\t\t-\t\t0x%lX\t\t| ", start, end);
            printf("RW-\n");
            curr_mini = curr_mini->next;
        }
        printf("Block %lu end\n", i + 1);

        curr_blck = curr_blck->next;
    }
}

void mprotect(arena_t *arena, uint64_t address, int8_t *permission)
{

}

