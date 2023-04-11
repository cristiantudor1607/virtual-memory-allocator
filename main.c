#include "vma.h"
#include "text_utils.h"
#include "funcs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MEMERR "Memory allocation error: not enough memory available.\n"

int main(void)
{
	arena_t *main_arena = NULL;
	char *command = NULL;
	int check;
    // memups este < 0 cand alocarea da fail, 0 daca trebuie sa iasa
    // din program, in mod normal, in urma comenzii de "exit",
    // sau > 0 daca se continua executia programului
	int memups = read_string(&command);
	while (memups > 0) {
		switch (cmd_idgen(command)) {
		case 1:
			ALLOC_ARENA_func(&main_arena, command, &memups);
			break;
		case 2:
			DEALLOC_ARENA_func(main_arena, command, &memups);
			break;
		case 3:
			ALLOC_BLOCK_func(&main_arena, command, &memups);
			break;
		case 4:
			FREE_BLOCK_func(main_arena, command, &memups);
			break;
		case 5:
			READ_func(main_arena, command, &memups);
			break;
		case 6:
			WRITE_func(main_arena, command, &memups);
			break;
		case 7:
			PMAP_func(main_arena, command, &memups);
			break;
		case 8:
			MPROTECT_func(main_arena, command, &memups);
			break;
		case 0:
			check = check_validity(command, 0);
			if (check == -1)
				memups = -1;
			break;
		default:
			// nu se ajunge in default, din cauza comenzii cmd_idgen
			break;
		}
		if (memups == -1 || memups == 0)
			break;
		memups = read_string(&command);
		if (memups < 0)
			break;
	}

	if (memups == -1) {
		printf(MEMERR);
		dealloc_arena(main_arena);
	}

	free(command);
	return 0;
}
