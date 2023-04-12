#include "vma.h"

// functie care cauta prin lista de blocuri
// daca noul bloc cu parametrii addr si size are loc in arena functia intoarce
// 0 daca nu are loc, 1 daca are loc si nu are alte blocuri adiacente, sau
// LEFT_ADJACENT, RIGHT_ADJACENT, sau BOTH_ADJACENT, daca are blocuri adiacente
// nota: cazul in care este in afara arenei sau depaseste dimensiunea arenei
// este deja verificat la momentul apelarii functiei
int find_place(arena_t *arena, uint64_t addr, size_t size)
{
	node_t *curr_node = arena->alloc_list->head;
	node_t *next_node = NULL;
	uint64_t curr_addr, next_addr;
	size_t curr_size;

	if (!curr_node)
		return 1;

	// testez daca ar avea loc la inceputul arenei
	curr_addr = ((block_t *)curr_node->data)->start_address;

	if (addr + size < curr_addr)
		return 1;

	if (addr + size == curr_addr)
		return RIGHT_ADJACENT;

	while (curr_node->next) {
		curr_addr = ((block_t *)curr_node->data)->start_address;
		curr_size = ((block_t *)curr_node->data)->size;

		next_node = curr_node->next;
		next_addr = ((block_t *)next_node->data)->start_address;

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

// functie care retunrneaza index-ul unde trebuie adaugat noul block
// functioneaza si pentru lista de blockuri, cat si pentru cea de miniblockuri
// la momentul apelarii functiei se stie deja ca are loc in lista
size_t get_position(list_t *list, uint64_t addr)
{
	node_t *curr = list->head;
	size_t idx = 0;
	uint64_t curr_addr = 0, next_addr = 0;

	if (!list->head)
		return 0;

	// testez daca trebuie adaugata pe prima pozitie
	if (list->data_size == sizeof(block_t))
		curr_addr = ((block_t *)curr->data)->start_address;
	else if (list->data_size == sizeof(miniblock_t))
		curr_addr = ((miniblock_t *)curr->data)->start_address;

	if (curr_addr > addr)
		return 0;

	while (curr->next) {
		idx++;
		if (list->data_size == sizeof(block_t)) {
			curr_addr = ((block_t *)curr->data)->start_address;
			next_addr = ((block_t *)curr->next->data)->start_address;
		} else if (list->data_size == sizeof(miniblock_t)) {
			curr_addr = ((miniblock_t *)curr->data)->start_address;
			next_addr = ((miniblock_t *)curr->next->data)->start_address;
		}

		if (curr_addr < addr && addr < next_addr)
			return idx;

		curr = curr->next;
	}

	idx++;

	// testez daca trebuie adaugat pe ultima pozitie
	if (list->data_size == sizeof(block_t))
		curr_addr = ((block_t *)curr->data)->start_address;
	else if (list->data_size == sizeof(miniblock_t))
		curr_addr = ((miniblock_t *)curr->data)->start_address;

	if (curr_addr < addr)
		return idx;

	// nu se ajunge niciodata sa nu se dea return la ceva pentru ca
	// verificarile au fost facute inainte, dar trebuie sa dau return
	// la ceva pentru warninguri
	return 0;
}

// functie care initiaza un miniblock
miniblock_t *init_miniblock(uint64_t address, size_t size, uint8_t perm)
{
	miniblock_t *new_mini = malloc(sizeof(miniblock_t));
	if (!new_mini)
		return NULL;

	new_mini->start_address = address;
	new_mini->size = size;
	new_mini->perm = perm;
	new_mini->rw_buffer = (int8_t *)calloc(size, sizeof(int8_t));
	if (!new_mini->rw_buffer) {
		free(new_mini);
		return NULL;
	}

	return new_mini;
}

void add_newblock(arena_t *arena, const uint64_t address, const uint64_t size)
{
	// creez un nou block ce trebuie adaugat in arena
	block_t *new_block = malloc(sizeof(block_t));
	if (!new_block) {
		arena->alloc_fail = -1;
		return;
	}

	new_block->size = size;
	new_block->start_address = address;

	// creez lista de miniblock-uri aferenta block-ului
	new_block->miniblock_list = create(sizeof(miniblock_t), free_miniblockdata);
	if (!new_block->miniblock_list) {
		free(new_block);
		arena->alloc_fail = -1;
		return;
	}

	// creez primul miniblock din lista de miniblock-uri, adica blockul
	//pe care il aloc propriu-zis
	miniblock_t *new_miniblock = init_miniblock(address, size, 6);
	if (!new_miniblock) {
		free_list(&new_block->miniblock_list);
		free(new_block);
		arena->alloc_fail = -1;
		return;
	}

	int8_t memups = 0;
	add_nthnode(new_block->miniblock_list, 0, new_miniblock, &memups);
	// pentru ca datele trimise ca parametru se copiaza in nod, trebuie sa dau
	// free
	free(new_miniblock);
	if (memups == -1) {
		free_list(&new_block->miniblock_list);
		free(new_block);
		arena->alloc_fail = -1;
		return;
	}

	// caut pozitia noului block in arena
	size_t idx = get_position(arena->alloc_list, address);
	add_nthnode(arena->alloc_list, idx, new_block, &memups);
	free(new_block);
	if (memups == -1) {
		free(new_miniblock);
		free_list(&new_block->miniblock_list);
		arena->alloc_fail = -1;
		return;
	}

	// scad din memoria libera a arenei
	arena->free_memory = arena->free_memory - size;
}

void mergeatright(arena_t *arena, const uint64_t address, const uint64_t size)
{
	miniblock_t *new_mini = init_miniblock(address, size, 6);
	if (!new_mini) {
		arena->alloc_fail = -1;
		return;
	}

	// gasesc index-ul block-ului in care trebuie adaugat miniblock-ul
	size_t block_idx = get_position(arena->alloc_list, address) - 1;
	node_t *block = get_nthnode(arena->alloc_list, block_idx);

	// modific doar dimensiunea blocului, deoarece daca adaug la dreapta, o
	// sa fie adaugat la final, si nu modifica adresa de inceput
	int8_t fail = 0;
	uint64_t idx = ((block_t *)block->data)->miniblock_list->size;
	list_t *list_to_add = ((block_t *)block->data)->miniblock_list;
	add_nthnode(list_to_add, idx, new_mini, &fail);
	free(new_mini);
	if (fail == -1) {
		arena->alloc_fail = -1;
		return;
	}

	((block_t *)block->data)->size += size;
	arena->free_memory = arena->free_memory - size;
}

void mergeatleft(arena_t *arena, const uint64_t addr, const uint64_t size)
{
	miniblock_t *new_mini = init_miniblock(addr, size, 6);
	if (!new_mini) {
		arena->alloc_fail = -1;
		return;
	}

	// gasesc index-ul blockului la care trebuie adaugat miniblockul
	uint64_t block_idx = get_position(arena->alloc_list, addr);
	node_t *block = get_nthnode(arena->alloc_list, block_idx);

	// daca ii dau merge left o sa fie adaugat pe prima pozitie
	int8_t fail = 0;
	list_t *list_to_add = ((block_t *)block->data)->miniblock_list;
	add_nthnode(list_to_add, 0, new_mini, &fail);
	free(new_mini);
	if (fail == -1) {
		arena->alloc_fail = -1;
		return;
	}

	((block_t *)block->data)->size += size;
	((block_t *)block->data)->start_address = addr;
	arena->free_memory = arena->free_memory - size;
}

void mergebothsides(arena_t *arena, const uint64_t addr, const size_t size)
{
	// as putea sa folosesc oricare dintre functiile de merge, pentru a adauga
	// un nod la stanga sau la dreapta, dar o sa o iau de la inceput, ca sa
	// nu caut de 2 ori pozitia prin lista

	miniblock_t *new_mini = init_miniblock(addr, size, 6);
	if (!new_mini) {
		arena->alloc_fail = -1;
		return;
	}

	uint64_t block_idx = get_position(arena->alloc_list, addr);
	node_t *block = get_nthnode(arena->alloc_list, block_idx);
	node_t *previous = block->prev;

	// adaug pe prima pozitie in bloc
	int8_t fail = 0;
	list_t *list_to_add = ((block_t *)block->data)->miniblock_list;
	add_nthnode(list_to_add, 0, new_mini, &fail);
	free(new_mini);
	if (fail == -1) {
		arena->alloc_fail = -1;
		return;
	}

	((block_t *)block->data)->size += size;
	((block_t *)block->data)->start_address = addr;
	arena->free_memory = arena->free_memory - size;

	((block_t *)previous->data)->size += ((block_t *)block->data)->size;
	mergelists(((block_t *)previous->data)->miniblock_list,
			   ((block_t *)block->data)->miniblock_list);

	// dau merge doar la liste-le din block-uri, nu la noduri, asa ca
	// eliberez memoria ocupata de nodul care nu mai contine nicio lista
	block = remove_nthnode(arena->alloc_list, block_idx);
	free((block_t *)block->data);
	free(block);
}

// functie care returneaza perechea (block, miniblock) pentru free
// fata de cea pt read si write, addr trebuie sa fie un inceput de miniblock
address_t *free_address(arena_t *arena, uint64_t addr)
{
	address_t *pair = malloc(sizeof(address_t));
	if (!pair) {
		arena->alloc_fail = -1;
		return NULL;
	}

	uint64_t block_idx = 0, mini_idx = 0;
	node_t *curr = arena->alloc_list->head;

	// daca nu exista niciun block returnez NULL
	if (!curr) {
		free(pair);
		return NULL;
	}

	while (curr) {
		uint64_t start = ((block_t *)curr->data)->start_address;
		uint64_t end = start + ((block_t *)curr->data)->size;

		if (start <= addr && addr <= end) {
			pair->block = curr;
			pair->b_idx = block_idx;

			node_t *aux = ((block_t *)curr->data)->miniblock_list->head;
			mini_idx = 0;
			while (aux) {
				if (addr == ((miniblock_t *)aux->data)->start_address) {
					pair->miniblock = aux;
					pair->m_idx = mini_idx;
					return pair;
				}

				aux = aux->next;
				mini_idx++;
			}

			// daca nu gaseste in block-ul curent, nu va gasi niciodata
			free(pair);
			return NULL;
		}

		curr = curr->next;
		block_idx++;
	}

	// daca adresa este din arena nu ajunge niciodata aici, dar trebuie :))
	free(pair);
	return NULL;
}

void delete_first(arena_t *arena, address_t *pair)
{
	node_t *block = pair->block;
	node_t *miniblock = pair->miniblock;
	// pastrez datele de care am nevoie din miniblock pentru a updata datele
	// blocului
	size_t size = ((miniblock_t *)miniblock->data)->size;

	list_t *list_to_remove = ((block_t *)block->data)->miniblock_list;
	node_t *remove = remove_nthnode(list_to_remove, 0);
	free(((miniblock_t *)remove->data)->rw_buffer);
	free(remove->data);
	free(remove);

	arena->free_memory += size;
	arena->minis_no--;
	((block_t *)block->data)->start_address += size;
	((block_t *)block->data)->size -= size;

	// daca e unicul miniblock trebuie sters tot blockul
	if (((block_t *)block->data)->size == 0) {
		remove = remove_nthnode(arena->alloc_list, pair->b_idx);

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

	// pastrez datele de care am nevoie din miniblock pentru a updata
	// informatiile blockului
	size_t size = ((miniblock_t *)miniblock->data)->size;
	uint64_t last = ((block_t *)block->data)->miniblock_list->size - 1;

	list_t *list_to_remove = ((block_t *)block->data)->miniblock_list;
	node_t *remove = remove_nthnode(list_to_remove, last);
	free(((miniblock_t *)remove->data)->rw_buffer);
	free(remove->data);
	free(remove);

	arena->free_memory += size;
	arena->minis_no--;

	// adresa de start nu se modifica, doar cea de end, dar e calculata
	// cu ajutorul lui size in alte functii
	((block_t *)block->data)->size -= size;

	// daca era unicul miniblock sterg blocul

	if (((block_t *)block->data)->size == 0) {
		remove = remove_nthnode(arena->alloc_list, pair->b_idx);
		list_t *list = ((block_t *)remove->data)->miniblock_list;
		free_list(&list);
		free(remove->data);
		free(remove);
	}
}

// functie care va fi apelata pentru determinarea noului size al unui block
// dupa operatia de free block
size_t get_size(list_t *miniblock_list)
{
	node_t *curr = miniblock_list->head;
	size_t size  = 0;
	while (curr) {
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
	if (!new_block) {
		arena->alloc_fail = -1;
		return;
	}

	new_block->miniblock_list = create(sizeof(miniblock_t), free_miniblockdata);
	if (!new_block->miniblock_list) {
		free(new_block);
		arena->alloc_fail = -1;
		return;
	}

	list_t *list = ((block_t *)block->data)->miniblock_list;
	node_t *new_head = miniblock->next;
	node_t *new_tail = miniblock->prev;

	// initializez lista
	// capetele noii liste
	new_block->miniblock_list->head = new_head;
	new_block->miniblock_list->tail = list->tail;

	// size-ul noii liste
	size_t init_size = list->size;
	new_block->miniblock_list->size = init_size - pair->m_idx - 1;

	// sterg nodul cu minibockul dorit
	node_t *remove = remove_nthnode(list, pair->m_idx);
	free(((miniblock_t *)remove->data)->rw_buffer);
	free(remove->data);
	free(remove);

	// acum new_tail este legat la new_head, si sparg aceasta legatura
	new_tail->next = NULL;
	list->tail = new_tail;
	list->size = pair->m_idx;
	new_head->prev = NULL;

	new_block->start_address = addr + size;
	new_block->size = get_size(new_block->miniblock_list);

	// mai trebuie facute schimbari si in blockul original
	((block_t *)block->data)->size -= (new_block->size + size);
	arena->free_memory += size;
	arena->minis_no--;
	// adaug noul block in lista, care este de fapt o copie a unei parti din
	// vechiul block
	int8_t memups = 0;
	add_nthnode(arena->alloc_list, pair->b_idx + 1, new_block, &memups);
	if (memups == -1) {
		free_list(&new_block->miniblock_list);
		free(new_block);
		arena->alloc_fail = -1;
		return;
	}

	free(new_block);
}

// functie care returneaza o pereche (block, miniblock) pentru read si write
address_t *read_write_addr(arena_t *arena, uint64_t addr)
{
	// cautarea este asemanatoare cu cea de la free_address, doar ca nu mai
	// exista conditia ca adresa sa fie inceputul unui miniblock
	address_t *pair = malloc(sizeof(address_t));
	if (!pair) {
		arena->alloc_fail = -1;
		return NULL;
	}

	node_t *curr_block = arena->alloc_list->head;
	uint64_t block_idx = 0;
	while (curr_block) {
		uint64_t start = ((block_t *)curr_block->data)->start_address;
		uint64_t end = start + ((block_t *)curr_block->data)->size;
		if (start <= addr && addr < end) {
			pair->b_idx = block_idx;
			pair->block = curr_block;

			list_t *list = ((block_t *)curr_block->data)->miniblock_list;
			node_t *curr_mini = list->head;
			uint64_t miniblock_idx = 0;
			while (curr_mini) {
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

// functie care scrie data incepand de la adresa addr, care se afla in
// miniblockul caracterizat de perechea pair, size bytes
void write_data(address_t *pair, uint64_t addr, int8_t *data, size_t size)
{
	// pastrez size-ul initial, deoarece o sa decrementez size
	size_t init_size = size;
	node_t *curr = pair->miniblock;
	miniblock_t *minblock = (miniblock_t *)curr->data;

	// bufferul este alocat deja de la initializare
	// bufferul va fi un sir de caractere s, iar octetii vor fi scrisi de la
	// pozitia s[start_idx], pentru a pastra corespondenta cu adresa
	uint64_t start_idx = addr - minblock->start_address;

	// iau 2 pointeri, cu buff_ptr ma plimb prin rw_buffer, iar cu data_ptr
	// prin data, si copiez caracter cu caracter, din data in buff
	int8_t *buff_ptr = (int8_t *)minblock->rw_buffer + start_idx;
	int8_t *data_ptr = data;
	uint64_t end = minblock->start_address + minblock->size;
	do {
		*buff_ptr = *data_ptr;
		data_ptr++;
		addr++;
		size--;
		// daca a ajuns la finalul unui miniblock
		if (addr == end) {
			// verific daca am ajuns la finalul block-ului, si daca se poate
			// trec mai departe, si mut pointer-ul in urm miniblock
			curr = curr->next;
			if (!curr)
				break;
			minblock = (miniblock_t *)curr->data;
			uint64_t buff_size = minblock->size;

			buff_ptr = (int8_t *)minblock->rw_buffer;
			end = minblock->start_address + buff_size;
			continue;
		}

		buff_ptr++;
	} while (size > 0 && curr);

	if (size > 0) {
		printf("Warning: size was bigger than the block size. ");
		printf("Writing %lu characters.\n", init_size - size);
	}
}

// functie care citeste data, de la adresa addr, din perechea pair, size bytes
void read_data(address_t *pair, uint64_t addr, size_t size)
{
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

	// la fel ca la write, parcurg caracter cu caracter rw_bufferul, cu
	// ajutorul unui pointer si printez caracterele
	uint64_t start_idx = addr - minblock->start_address;
	int8_t *buff_ptr = (int8_t *)minblock->rw_buffer + start_idx;

	uint64_t end = minblock->start_address + minblock->size;

	do {
		if (*buff_ptr != '\0')
			printf("%c", *buff_ptr);
		addr++;
		size--;
		if (addr == end) {
			// trec pe urmatorul miniblock, sau ies din loop, pentru ca nu mai
			// am de unde sa citesc
			curr = curr->next;
			if (!curr)
				break;

			minblock = (miniblock_t *)curr->data;
			buff_ptr = (int8_t *)minblock->rw_buffer;
			end = minblock->start_address + minblock->size;
			continue;
		}

		buff_ptr++;

	} while (size > 0 && curr);

	printf("\n");
}

// functie care transforma o permisiune data numeric, intr-o structura de
// permisiune definita in structures.h
permission_t check_perm(int8_t perm)
{
	permission_t my_perms;
	my_perms.execute = 0;
	my_perms.write = 0;
	my_perms.read = 0;

	if (perm - 4 >= 0) {
		perm = perm - 4;
		my_perms.read = 1;
	}

	if (perm - 2 >= 0) {
		perm = perm - 2;
		my_perms.write = 1;
	}

	if (perm - 1 >= 0) {
		perm = perm - 1;
		my_perms.execute = 1;
	}

	return my_perms;
}

// functie care verifica daca am permisiuni de read sau write in toata bucata
// de block in care vreau sa scriu sau sa citesc
int8_t block_perms(address_t *pair, size_t size, int8_t perm)
{
	node_t *curr = pair->miniblock;
	miniblock_t *mini = (miniblock_t *)pair->miniblock->data;
	int8_t curr_perms = mini->perm;

	do {
		permission_t this_perms = check_perm(curr_perms);
		if (perm == 4) {
			if (this_perms.read == 0)
				return 0;
		}

		if (perm == 2) {
			if (this_perms.write == 0)
				return 0;
		}

		size = size - mini->size;
		if (!curr->next)
			break;

		curr = curr->next;
		mini = (miniblock_t *)curr->data;
		curr_perms = mini->perm;
	} while (size > 0);

	return 1;
}

// functie care printeaza permisiunile
void print_perms(int8_t perm)
{
	if (perm - 4 >= 0) {
		printf("R");
		perm = perm - 4;
	} else {
		printf("-");
	}

	if (perm - 2 >= 0) {
		printf("W");
		perm = perm - 2;
	} else {
		printf("-");
	}

	if (perm - 1 >= 0) {
		printf("X");
		perm = perm - 1;
	} else {
		printf("-");
	}
}

arena_t *alloc_arena(const uint64_t size)
{
	arena_t *new_arena = (arena_t *)malloc(sizeof(arena_t));
	if (!new_arena)
		return NULL;

	new_arena->arena_size = size;
	new_arena->free_memory = size;
	new_arena->alloc_fail = 1;
	new_arena->minis_no = 0;
	new_arena->alloc_list = create(sizeof(block_t), free_blockdata);
	if (!new_arena->alloc_list) {
		new_arena->alloc_fail = -1;
		return NULL;
	}

	return new_arena;
}

void dealloc_arena(arena_t *arena)
{
	if (!arena)
		return;
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
		add_newblock(arena, address, size);
		arena->minis_no++;
		return;
	}

	// daca are adiacent la stanga, trebuie sa dau merge la dreapta
	if (put == LEFT_ADJACENT) {
		mergeatright(arena, address, size);
		arena->minis_no++;
		return;
	}

	// daca are adiacent la dreapta, trebuie sa dau merge la stanga
	if (put == RIGHT_ADJACENT) {
		mergeatleft(arena, address, size);
		arena->minis_no++;
		return;
	}

	// merge in ambele parti
	if (put == BOTH_ADJACENT) {
		mergebothsides(arena, address, size);
		arena->minis_no++;
		return;
	}
}

void free_block(arena_t *arena, const uint64_t address)
{
	address_t *pair = free_address(arena, address);
	if (arena->alloc_fail == -1)
		return;

	if (!pair) {
		printf("Invalid address for free.\n");
		return;
	}

	// daca e la inceputul unui miniblock
	if (pair->m_idx == 0) {
		delete_first(arena, pair);
		free(pair);
		return;
	}

	// daca e la sfarsitul unui miniblock
	size_t list_size = ((block_t *)pair->block->data)->miniblock_list->size;
	if (pair->m_idx == list_size - 1) {
		delete_last(arena, pair);
		free(pair);
		return;
	}

	// daca e in interiorul unui miniblock
	delete_inside(arena, pair);
	free(pair);
}

void read(arena_t *arena, uint64_t address, uint64_t size)
{
	address_t *pair = read_write_addr(arena, address);

	// intai testez daca NULL a fost returnat pentru ca alocarea a dat fail
	if (arena->alloc_fail == -1)
		return;

	if (!pair) {
		printf("Invalid address for read.\n");
		return;
	}

	// verific permisiunile de citire in cate miniblockuri am nevoie
	int8_t is_ok = block_perms(pair, size, 4);
	if (is_ok == 0) {
		printf("Invalid permissions for read.\n");
		free(pair);
		return;
	}

	// citesc datele
	read_data(pair, address, size);
	free(pair);
}

void write(arena_t *arena, const uint64_t address,
		   const uint64_t size, int8_t *data)
{
	// obtin blocul si miniblocul de unde incep sa scriu
	address_t *pair = read_write_addr(arena, address);
	if (arena->alloc_fail == -1)
		return;

	// cazul in care nu s-a gasit
	if (!pair) {
		printf("Invalid address for write.\n");
		return;
	}

	// verific daca are permisiuni de scriere
	int8_t is_ok = block_perms(pair, size, 2);
	if (is_ok == 0) {
		printf("Invalid permissions for write.\n");
		free(pair);
		return;
	}

	write_data(pair, address, data, size);
	free(pair);
}

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
		// pe care le - am folosit pentru block-uri, iar pentru numarul de
		// noduri din liste pot sa - l folosesc pe minis_no
		minis_no = ((block_t *)curr_blck->data)->miniblock_list->size;
		for (uint64_t j = 0; j < minis_no; j++) {
			start = ((miniblock_t *)curr_mini->data)->start_address;
			end = start + ((miniblock_t *)curr_mini->data)->size;

			printf("Miniblock %lu:", j + 1);
			printf("\t\t0x%lX\t\t-\t\t0x%lX\t\t| ", start, end);

			int8_t perm = ((miniblock_t *)curr_mini->data)->perm;
			print_perms(perm);
			printf("\n");

			curr_mini = curr_mini->next;
		}

		printf("Block %lu end\n", i + 1);

		curr_blck = curr_blck->next;
	}
}

void mprotect(arena_t *arena, uint64_t address, int8_t *permission)
{
	// adresa a fost deja verificata la momentul apelarii functiei
	address_t *pair = free_address(arena, address);
	miniblock_t *mini = (miniblock_t *)pair->miniblock->data;
	mini->perm = *permission;
	free(pair);
}

