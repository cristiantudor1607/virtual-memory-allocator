#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define INCREASE 10

int read_string(char **string);
int check_validity(char *command, size_t args);
char *get_nth_arg(char *command, size_t n);
int cmd_idgen(char *command);
void print_error(char *msg, size_t n);
char *get_bytes(char *command, size_t addr_len, size_t size_len);
char *complete_arg(char *bytes, size_t size);