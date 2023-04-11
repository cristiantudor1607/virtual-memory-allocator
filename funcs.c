#include "funcs.h"

void ALLOC_ARENA_func(arena_t **arena, char *command, int *memups)
{
	int check = check_validity(command, 2);

	if (check == -1) {
		*memups = -1;
		return;
	}

	if (check == 0)
		return;

	char *str_size = get_nth_arg(command, 2);
	if (!str_size) {
		*memups = -1;
		return;
	}

	// strtoul returneaza 0 daca nu s-a putut face o conversie valida
	uint64_t size = strtoul(str_size, NULL, 10);
	free(str_size);

	// size-ul nu are cum sa fie 0, asa ca verific daca s-a facut conversia
	if (size == 0) {
		// dau comanda de check la un nr mare de argumente, ca sa afisez
		// invalid command-urile
		check_validity(command, BIG_NUM);
		return;
	}

	*arena = alloc_arena(size);
	if (!(*arena)) {
		*memups = -1;
		return;
	}
}

void DEALLOC_ARENA_func(arena_t *arena, char *command, int *memups)
{
	int check = check_validity(command, 1);

	if (check == -1) {
		*memups = -1;
		return;
	}

	if (check == 0)
		return;

	dealloc_arena(arena);
	*memups = 0;
}

void ALLOC_BLOCK_func(arena_t **arena, char *command, int *memups)
{
	int check = check_validity(command, 3);
	if (check == -1) {
		*memups = -1;
		return;
	}

	if (check == 0)
		return;

	char *str_addr = get_nth_arg(command, 2);
	if (!str_addr) {
		*memups = -1;
		return;
	}

	uint64_t addr = strtoul(str_addr, NULL, 10);
	size_t k = strcmp(str_addr, "0");
	free(str_addr);

	// verific daca s-a facut conversia, adica user-ul sa nu fi scris un string
	// care nu poate fi convertit
	if (addr == 0 && k != 0) {
		check_validity(command, BIG_NUM);
		return;
	}

	char *str_size = get_nth_arg(command, 3);
	if (!str_size) {
		*memups = -1;
		return;
	}

	size_t size = strtoul(str_size, NULL, 10);
	free(str_size);

	// aceeasi verificare ca cea de mai sus
	if (str_size == 0) {
		check_validity(command, BIG_NUM);
		return;
	}

	// acum verific daca block-ul poate fi pus in lista
	if (addr >= (*arena)->arena_size) {
		printf("The allocated address is outside the size of arena\n");
		return;
	}

	if (addr + size > (*arena)->arena_size) {
		printf("The end address is past the size of the arena\n");
		return;
	}

	alloc_block(*arena, addr, size);
	// verific daca a existat un fail in timpul alocarii blocului
	if ((*arena)->alloc_fail == -1) {
		*memups = -1;
		return;
	}
}

void FREE_BLOCK_func(arena_t *arena, char *command, int *memups)
{
	int check = check_validity(command, 2);
	if (check == -1) {
		*memups = -1;
		return;
	}

	if (check == 0)
		return;

	char *str_addr = get_nth_arg(command, 2);
	if (!str_addr) {
		*memups = -1;
		return;
	}

	uint64_t addr = strtoul(str_addr, NULL, 10);
	size_t k = strcmp(str_addr, "0");
	free(str_addr);
	if (addr == 0 && k != 0) {
		*memups = -1;
		return;
	}

	free_block(arena, addr);
	// verific daca a existat vreun fail la resursele alocate in procesul
	// de free
	if (arena->alloc_fail == -1) {
		*memups = -1;
		return;
	}
}

void WRITE_func(arena_t *arena, char *command, int *memups)
{
	// in cazul write nu mai incep cu validarea clasica, folosita la
	// restul comenzilor, deoarece are un numar variabil de parametrii
	// separati prin spatiu
	char *addr_str = get_nth_arg(command, 2);
	if (!addr_str) {
		*memups = -1;
		return;
	}

	// daca nu exista al doilea argument, comanda get_nth_arg, il seteaza pe
	// addr_str la inceputul sirului command
	if (addr_str == command) {
		printf("Invalid command.Please try again.\n");
		return;
	}

	// salvez lungimea ca sa sar peste addr_len caractere, in functia
	// get_bytes
	size_t addr_len = strlen(addr_str);

	uint64_t addr = strtoul(addr_str, NULL, 10);
	size_t k = strcmp(addr_str, "0");
	free(addr_str);

	// daca al treilea argument nu este unul valid
	if (addr == 0 && k != 0) {
		check_validity(command, 0);
		return;
	}

	char *size_str = get_nth_arg(command, 3);
	if (!size_str) {
		*memups = -1;
		return;
	}

	// daca nu exista al treilea argument (asem. cu prima testare de mai sus)
	if (size_str == command) {
		printf("Invalid command.Please try again.\n");
		printf("Invalid command.Please try again.\n");
		return;
	}

	// salvez lungimea pt a apela functia de get_bytes
	size_t size_len = strlen(size_str);

	size_t size = strtoul(size_str, NULL, 10);
	free(size_str);
	// size nu ar avea cum sa fie 0, asa ca nu mai pun conditia suplimentara
	if (size == 0) {
		check_validity(command, 0);
		return;
	}

	char *bytes = get_bytes(command, addr_len, size_len);
	int8_t *data = NULL;

	// iau 2 cazuri separate, daca se apasa '\n' dupa comanda, sau daca sunt
	// cativa bytes scrisi
	if (*bytes == '\0')
		data = (int8_t *)read_chars(size);
	else
		data = (int8_t *)complete_arg(bytes, size);

	write(arena, addr, size, data);
	free(data);
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
	if (!str) {
		*memups = -1;
		return;
	}

	uint64_t addr = strtoul(str, NULL, 10);
	size_t k = strcmp(str, "0");
	free(str);
	if (addr == 0 && k != 0) {
		*memups = -1;
		return;
	}

	str = get_nth_arg(command, 3);
	if (!str) {
		*memups = -1;
		return;
	}

	uint64_t size = strtoul(str, NULL, 10);
	free(str);
	if (size == 0) {
		*memups = -1;
		return;
	}

	read(arena, addr, size);
}

void PMAP_func(arena_t *arena, char *command, int *memups)
{
	int check = check_validity(command, 1);
	if (check == -1) {
		*memups = -1;
		return;
	}

	if (check == 0)
		return;

	pmap(arena);
}

void MPROTECT_func(arena_t *arena, char *command, int *memups)
{
	// MPROTECT poate avea un numar variabil de parametrii, asa ca, asem.
	// cazului WRITE, nu merge nici aici verificarea clasica
	char *addr_str = get_nth_arg(command, 2);
	if (!addr_str) {
		*memups = -1;
		return;
	}

	uint64_t addr = strtoul(addr_str, NULL, 10);
	size_t k = strcmp(addr_str, "0");
	free(addr_str);
	if (addr == 0 && k != 0) {
		*memups = -1;
		return;
	}

	// caut perechea (block, miniblock) in arena, iar daca nu o gasesc,
	// adresa nu e valida pentru mprotect (se poate aplica doar pe adresa de
	// inceput de miniblock, exact ca la free)
	address_t *pair = free_address(arena, addr);
	if (!pair) {
		printf("Invalid address for mprotect.\n");
		return;
	}
	free(pair);

	int8_t perm = get_permissions(command);
	if (perm == -1)
		return;

	mprotect(arena, addr, &perm);
}
