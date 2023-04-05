#include "DLL.h"

// functie care creaza o lista dublu inaluntita in care informatiile din nod au
// data_size bytes
list_t *create(uint64_t data_size, void (*free_func)(void *))
{
    list_t *my_list = malloc(sizeof(list_t));
    if (my_list == NULL)
        return NULL;

    my_list->head = NULL;
    my_list->tail = NULL;
    my_list->size = 0;
    my_list->data_size = data_size;
    my_list->free_func = free_func;

    return my_list;
}

// functie care returneaza nodul n din lista
// daca n depaseste size - ul listei returnez NULL
node_t *get_nthNode(list_t *list, uint64_t n)
{
    if (n >= list->size)
        return NULL;

    node_t *curr = list->head;
    for (uint64_t i = 0; i < n; i++) {
        curr = curr->next;
    }

    return curr;
}

// am adaugat paramterul fail pentru a tine cont daca alocarile
// au esuat
void add_nthNode(list_t *list, uint64_t n, const void *data, int8_t *fail)
{
    node_t *new_node = malloc(sizeof(node_t));
    if (new_node == NULL) {
        *fail = -1;
        return;
    }

    new_node->data = malloc(list->data_size);
    if (new_node->data == NULL) {
        *fail = -1;
        return;
    }

    memcpy(new_node->data, data, list->data_size);
    
    if (list->size == 0) {
        new_node->prev = NULL;
        new_node->next = NULL;
        list->head = new_node;
        list->tail = new_node;
        list->size++;
        return;
    }

    if (n == 0) {
        new_node->prev = NULL;
        new_node->next = list->head;
        list->head->prev = new_node;
        list->head = new_node;
        list->size++;
        return;
    }

    if (n >= list->size) {
        new_node->next = NULL;
        new_node->prev = list->tail;
        list->tail->next = new_node;
        list->tail = new_node;
        list->size++;
        return;
    }

    node_t *curr = list->head;
    for (uint64_t i = 0; i < n - 1; i++)
        curr = curr->next;

    // curr este nodul de dinaintea celui pe care il adaug

    new_node->next = curr->next;
    new_node->prev = curr;
    curr->next->prev = new_node;
    curr->next = new_node;
    list->size++;
}

node_t *remove_firstNode(list_t *list)
{
    node_t *ret_node = list->head;
    if (list->size == 1) {
        list->head = NULL;
        list->tail = NULL;
        list->size--;
        return ret_node;
    }

    list->head = list->head->next;
    list->head->prev = NULL;
    list->size--;
    return ret_node;
}

node_t *remove_lastNode(list_t *list)
{
    node_t *ret_node = list->tail;
    if (list->size == 1) {
        list->tail = NULL;
        list->head = NULL;
        list->size--;
        return ret_node;
    }

    list->tail = list->tail->prev;
    list->tail->next = NULL;
    list->size--;
    return ret_node;
}

// functia reface legaturile in lista si intoarce nodul ce trebuie sters
node_t *remove_nthNode(list_t *list, uint64_t n)
{
    if (n >= list->size)
        return NULL;

    node_t *node;
    if (n == 0) {
        node = remove_firstNode(list);
        return node;
    }

    if (n == list->size - 1) {
        node = remove_lastNode(list);
        return node;
    }

    node_t *aux = list->head;
    for (uint64_t i = 0; i < n - 1; i++)
        aux = aux->next;

    // salvez in pointerul node, nodul care trebuie sters
    node = aux->next;
    node->next->prev = aux;
    aux->next = node->next;
    list->size--;
    return node;
}

// data : curr->data
// data contine un miniblock, care la randul lui contine un buffer
void free_miniblockData(void *data)
{
    free(((miniblock_t *)data)->rw_buffer);
    free((miniblock_t *)data);
}

// data : curr->data
// data contine un block, care la randul ei contine o lista de miniblock - uri
void free_blockData(void *data) {
    block_t *block = (block_t *)data;
    free_list(&block->miniblock_list);
    free(block);
}

void free_list(list_t **list)
{
    if (*list == NULL)
        return;

    uint64_t n = (*list)->size;
    node_t *curr;
    for (uint64_t i = 0; i < n; i++) {
        curr = remove_nthNode(*list, 0);
        (*list)->free_func(curr->data);
        free(curr);
    }

    free(*list);
    *list = NULL;
}

void free_int(void *data)
{
    free((int *)data);
}

void mergeLists(list_t *head_list, list_t *tail_list)
{
    tail_list->head->prev = head_list->tail;
    head_list->tail->next = tail_list->head;
    head_list->tail = tail_list->tail;
    head_list->size = head_list->size + tail_list->size;
    free(tail_list);
}