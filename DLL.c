#include "DLL.h"

// functie care creaza o lista dublu inaluntita in care informatiile din nod au
// data_size bytes, si i se asociaza functia de free "free_func", pentru a
// elibera data din noduri
list_t *create(uint64_t data_size, void (*free_func)(void *))
{
	list_t *my_list = malloc(sizeof(list_t));
	if (!my_list)
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
node_t *get_nthnode(list_t *list, uint64_t n)
{
	if (n >= list->size)
		return NULL;

	node_t *curr = list->head;
	for (uint64_t i = 0; i < n; i++)
		curr = curr->next;

	return curr;
}

// am adaugat paramterul fail pentru a tine cont daca alocarea lui data
// esueaza, pentru a transmite mai departe in main, ca sa inchei executia
// programului
void add_nthnode(list_t *list, uint64_t n, const void *data, int8_t *fail)
{
	node_t *new_node = malloc(sizeof(node_t));
	if (!new_node) {
		*fail = -1;
		return;
	}

	new_node->data = malloc(list->data_size);
	if (!new_node->data) {
		free(new_node);
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

node_t *remove_firstnode(list_t *list)
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

node_t *remove_lastnode(list_t *list)
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
node_t *remove_nthnode(list_t *list, uint64_t n)
{
	if (n >= list->size)
		return NULL;

	node_t *node;
	if (n == 0) {
		node = remove_firstnode(list);
		return node;
	}

	if (n == list->size - 1) {
		node = remove_lastnode(list);
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

// functie care elibereaza data dintr-un nod ce contine un miniblock
void free_miniblockdata(void *data)
{
	free(((miniblock_t *)data)->rw_buffer);
	free((miniblock_t *)data);
}

// functie care elibereaza data dintr-un nod ce contine un block
void free_blockdata(void *data)
{
	block_t *block = (block_t *)data;
	free_list(&block->miniblock_list);
	free(block);
}

void free_list(list_t **list)
{
	if (!(*list))
		return;

	uint64_t n = (*list)->size;
	node_t *curr;
	for (uint64_t i = 0; i < n; i++) {
		curr = remove_nthnode(*list, 0);
		(*list)->free_func(curr->data);
		free(curr);
	}

	free(*list);
	*list = NULL;
}

// functie care lipeste pe tail_list la sfarsitul lui head_list
void mergelists(list_t *head_list, list_t *tail_list)
{
	tail_list->head->prev = head_list->tail;
	head_list->tail->next = tail_list->head;
	head_list->tail = tail_list->tail;
	head_list->size = head_list->size + tail_list->size;
	free(tail_list);
}
