#include "text_utils.h"

// string - urile sunt citite caracter cu caracter si alocate dinamic,
// iar functia returneaza un int > 0 daca s - a reusit alocarea totala
// a string - ului, sau un int < 0, daca nu s - a reusit citirea
// string - ului
int read_string(char **string) {
    
    // daca string - ul este deja alocat vreau sa ii dau free
    if (*string != NULL)
        free(*string);

    char c;
    int capacity = INCREASE;
    int size = 0;
    *string = (char *)malloc(capacity * sizeof(char));
    if (*string  == NULL)
        return -1;

    scanf("%c", &c);

    // vreau sa ignor daca se citesc '\n' - uri la inceput sau spatii
    while (c == '\n' || c == ' ') {
        scanf("%c", &c);
    }

    do {
        if (size >= capacity) {
            capacity += INCREASE;
            *string = (char *)realloc(*string, capacity * sizeof(char));
            if (*string == NULL)
                return -1;
        }

        *(*string + size) = c;
        size++;
        scanf("%c", &c);
    } while (c != '\n');

    *string = (char *)realloc(*string, (size + 1) * sizeof(char));
    if (*string == NULL)
        return -1;

    *(*string + size) = '\0';

    return size;
}

// functia afiseaza mesajul de n ori
void print_error(char *msg, size_t n)
{
    for (size_t i = 0; i < n; i++)
        printf("%s", msg);
}

// functie care verifica daca numarul de argumente din comanda este corect
// functia returneaza 0 daca nr de paramterii nu este corect, 1 daca este
// corect, sau -1 daca a intampinat probleme cu memoria
int check_validity(char *command, size_t args)
{
    // cazul in care alocarea dinamica a sirului de caractere nu s-a reusit
    if (command == NULL)
        return -1;
    
    char *copy = (char *)malloc((strlen(command) + 1) * sizeof(char));
    if (copy == NULL)
        return -1;
    
    memcpy(copy, command, (strlen(command) + 1) * sizeof(char));
    size_t args_num = 0;
    char *p = strtok(copy, " ");

    while (p) {
        args_num++;
        p = strtok(NULL, " ");
    }

    free(copy);
    // o sa adaug mesajul de eroare
    if (args_num != args) {
        print_error("Invalid command. Please try again.\n", args_num);
        return 0;
    }

    return 1;
}

// functia intoarce argumentul in forma de string,
// sau NULL in caz de fail de alocare
char *get_nth_arg(char *command, size_t n)
{
    // cazul in care alocarea sirului nu a reusit
    if (command == NULL)
        return NULL;

    char *copy = (char *)malloc((strlen(command) + 1) * sizeof(char));
    if (copy == NULL)
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
    
    // ma intereseaza cazul in care numarul de argumente este prea mic
    // deoarce trimit NULL cand nu se reuseste alocarea am nevoie de o
    // alta conventie
    // ca sa nu mai fac o alocare, setez pointerul copy la inceputul lui
    // command
    copy = command;
    if (argC < n)
        return copy;
    return res_value;
}

// functie care ii atribuie fiecarei comenzi un id unic
int cmd_idgen(char *command)
{
    // pentru cazurile in care alocarea dinamica esueaza, id - ul este -1
    if (command == NULL)
        return -1;

    char *keyword = get_nth_arg(command, 1);
    if (keyword == NULL)
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

    free(keyword);
    // pentru comenzile invalide id - ul o sa fie 0
    return 0;
}

// se va face verificare daca este comanda valida
//inainte de apelarea functiei
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

char *complete_arg(char *bytes, size_t size)
{
    char *str = (char *)malloc((size + 1) * sizeof(char));
    if (str == NULL)
        return NULL;

    size_t k = strlen(bytes);
    memcpy(str, bytes, k * sizeof(char));
    str[k] = '\n';
    k++;
    char c = 'a';
    while (c != '\n') {
        scanf("%c", &c);
        if (k >= size)
            continue;
        str[k] = c;
        k++;
    }
    str[size] = '\0';
    return str;

}
