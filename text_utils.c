// Copyright Tudor Cristian-Andrei 311CAa 2022-2023
#include "text_utils.h"

// string - urile sunt citite caracter cu caracter si alocate dinamic,
// iar functia returneaza un int > 0 daca s-a reusit alocarea totala
// a string-ului, sau un int < 0, daca nu s-a reusit citirea string-ului
int read_string(char **string)
{
	// daca string-ul este deja alocat vreau sa ii dau free
	if (*string)
		free(*string);

	char c;
	int capacity = INCREASE;
	int size = 0;
	*string = (char *)malloc(capacity * sizeof(char));
	if (!(*string))
		return -1;

	scanf("%c", &c);

	// vreau sa ignor daca se citesc '\n' - uri la inceput sau spatii
	while (c == '\n' || c == ' ')
		scanf("%c", &c);

	do {
		if (size >= capacity) {
			capacity += INCREASE;
			*string = (char *)realloc(*string, capacity * sizeof(char));
			if (!(*string))
				return -1;
		}

		*(*string + size) = c;
		size++;
		scanf("%c", &c);
	} while (c != '\n');

	// las string-ul la dimensiunea lui
	// nota: este putin probabil sa dea fail aceasta realocare, dar totusi
	// trebuie sa verific
	*string = (char *)realloc(*string, (size + 1) * sizeof(char));
	if (!(*string))
		return -1;

	// pun \0 la final
	*(*string + size) = '\0';

	return size;
}

// functia afiseaza mesajul din argument de n ori
// (folosita pt invalid commands)
void print_error(char *msg, size_t n)
{
	for (size_t i = 0; i < n; i++)
		printf("%s", msg);
}

// functie care verifica daca numarul de argumente din comanda este corect
// functia returneaza 0 daca nr de paramterii nu este corect, 1 daca este
// corect, sau -1 daca s-au intampinat probleme cu alocarea de memorie
int check_validity(char *command, size_t args)
{
	// cazul in care alocarea dinamica a sirului de caractere nu s-a reusit
	// si se apleaza totusi functia
	if (!command)
		return -1;

	// creez o copie ca sa nu stric string-ul (pt cazurile in care va mai fi
	// apleata alta comanda care il foloseste)
	char *copy = (char *)malloc((strlen(command) + 1) * sizeof(char));
	if (!copy)
		return -1;

	memcpy(copy, command, (strlen(command) + 1) * sizeof(char));

	// numar cate argumente separate prin spatiu exista
	size_t args_num = 0;
	char *p = strtok(copy, " ");
	while (p) {
		args_num++;
		p = strtok(NULL, " ");
	}

	free(copy);

	if (args_num != args) {
		print_error("Invalid command. Please try again.\n", args_num);
		return 0;
	}

	return 1;
}

// functie care intoarce al n-lea argument (considerand ca argumentele sunt
// separate prin spatii)
char *get_nth_arg(char *command, size_t n)
{
	// cazul in care alocarea sirului nu a reusit
	if (!command)
		return NULL;

	// creez iar o copie ca sa nu stric string-ul
	char *copy = (char *)malloc((strlen(command) + 1) * sizeof(char));
	if (!copy)
		return NULL;

	memcpy(copy, command, (strlen(command) + 1) * sizeof(char));
	size_t argC = 0;

	char *res_value = NULL;
	char *p = strtok(copy, " ");
	while (p) {
		argC++;
		if (argC == n) {
			res_value = (char *)malloc((strlen(p) + 1) * sizeof(char));
			memcpy(res_value, p, (strlen(p) + 1) * sizeof(char));
			break;
		}

		p = strtok(NULL, " ");
	}

	free(copy);

	// ma intereseaza si cazul in care numarul de argumente este prea mic
	// am nevoie de alta conventie, deoarece NULL este trims la fail-urile de
	// alocare
	// cand nr de argumente este prea mic returnez un pointer catre inceputul
	// lui command
	copy = command;
	if (argC < n)
		return copy;

	// returnez argumentul sub forma de string
	return res_value;
}

// functie care ii atribuie fiecarei comenzi un id unic
int cmd_idgen(char *command)
{
	// pentru cazurile in care alocarea dinamica esueaza, id - ul este -1
	if (!command)
		return -1;

	char *keyword = get_nth_arg(command, 1);
	if (!keyword)
		return -1;

	if (strcmp(keyword, "ALLOC_ARENA") == 0) {
		free(keyword);
		return 1;
	}

	if (strcmp(keyword, "DEALLOC_ARENA") == 0) {
		free(keyword);
		return 2;
	}

	if (strcmp(keyword, "ALLOC_BLOCK") == 0) {
		free(keyword);
		return 3;
	}

	if (strcmp(keyword, "FREE_BLOCK") == 0) {
		free(keyword);
		return 4;
	}

	if (strcmp(keyword, "READ") == 0) {
		free(keyword);
		return 5;
	}

	if (strcmp(keyword, "WRITE") == 0) {
		free(keyword);
		return 6;
	}

	if (strcmp(keyword, "PMAP") == 0) {
		free(keyword);
		return 7;
	}

	if (strcmp(keyword, "MPROTECT") == 0) {
		free(keyword);
		return 8;
	}

	free(keyword);

	// pentru comenzile invalide id - ul o sa fie 0
	return 0;
}

// functie care imi seteaza un pointer la inceputul sirului pe care trebuie
// sa-l scriu in buffer, care va fi apelata doar daca acesta exista scrisa
// dupa WRITE, adresa si size
char *get_bytes(char *command, size_t addr_len, size_t size_len)
{
	char *ptr = command;
	// sar peste octetii lui WRITE
	ptr = ptr + 5;

	// sar peste spatii
	while (*ptr == ' ')
		ptr = ptr + 1;

	// sar peste al doilea argument
	ptr = ptr + addr_len;

	// sar iar peste spatii
	while (*ptr == ' ')
		ptr = ptr + 1;

	// sar peste al treilea argument
	ptr = ptr + size_len;

	// sar si peste ultimele spatii
	while (*ptr == ' ')
		ptr = ptr + 1;

	return ptr;
}

// functie care imi completeaza sirul deja dat, pana la dimensiunea de
// size octeti (care va fi trimis ca "data" la functia write)
char *complete_arg(char *bytes, size_t size)
{
	// vreau sa salvez totul intr-un nou sir
	char *str = malloc((size + 1) * sizeof(char));
	if (!str)
		return NULL;

	// copiez ce am deja, iar daca depaseste, doar size octeti
	size_t k = strlen(bytes);
	if (k > size) {
		memcpy(str, bytes, size * sizeof(char));
		str[size] = '\0';
		return str;
	}

	// altfel copiez k octeti si dupa completez sirul
	memcpy(str, bytes, k * sizeof(char));

	// daca este "fix pe fix" pun terminatorul de sir si returnez asa
	if (k == size) {
		str[k] = '\0';
		return str;
	}

	// daca se ajunge aici, mai trebuie citit
	// pun si un '\n', care a fost introdus de la tastatura, dar din cauza
	// comenzii de read_string, il ignor
	str[k] = '\n';
	k++;
	size = size - k;

	char character = '\n';
	while (size > 0 || character != '\n') {
		scanf("%c", &character);
		// daca am pus cat trebuie in sir, continui sa citesc pana se apasa \n
		if (size == 0)
			continue;

		str[k] = character;
		k++;
		size--;
	}

	str[k] = '\0';
	return str;
}

// functie care se apleaza la o comanda de forma WRITE adresa dimensiune \n
int8_t *read_chars(size_t size)
{
	int8_t *str = malloc((size + 1) * sizeof(int8_t));
	if (!str)
		return NULL;

	// pun un \n la inceputul sirului, care a fost ignorat la citirea comenzii
	str[0] = '\n';
	size--;

	// k serveste ca index pentru sir (str[k])
	size_t k = 1;
	int8_t character = 'a';
	do {
		scanf("%c", &character);
		// ca la cealalta functie, daca se depaseste size-ul, continui sa
		// citesc pana se apasa enter
		if (size == 0)
			continue;

		str[k] = character;
		k++;
		size--;
	} while (size > 0 || character != '\n');

	str[k] = '\0';
	return str;
}

// asemanator functiei care imi genereaza un id pt fiecare comanda,
// perm_id returneaza octalul fiecarei permisiuni
int8_t perm_id(char *keyword)
{
	if (strcmp(keyword, "PROT_NONE") == 0)
		return 0;

	if (strcmp(keyword, "PROT_READ") == 0)
		return 4;

	if (strcmp(keyword, "PROT_WRITE") == 0)
		return 2;

	if (strcmp(keyword, "PROT_EXEC") == 0)
		return 1;

	return -1;
}

int8_t get_permissions(char *command)
{
	// nu mai fac copie a string-ului, pentru ca nu o sa mai am nevoie de el

	// prima oara trec peste MPROTECT
	char *p = strtok(command, " |");
	size_t count = 1;

	// cu error verific daca vreunul dintre argumente este string invalid
	int error = 0;

	// trec peste adresa
	p = strtok(NULL, " |");
	count++;

	p = strtok(NULL, " |");
	int8_t perm = 0;
	while (p) {
		count++;
		int8_t this_perm = perm_id(p);
		if (this_perm == -1) {
			error = 1;
			p = strtok(NULL, " |");
			continue;
		}

		perm += this_perm;
		p = strtok(NULL, " |");
	}

	if (error == 1) {
		print_error("Invalid command. Please try again.\n", count);
		return -1;
	}

	return perm;
}
