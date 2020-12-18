#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>

#define MAX_SIZE_DICTIONARY 3838

typedef unsigned char byte_t;

typedef struct {
    char *entries[MAX_SIZE_DICTIONARY];
    int nb_entries;
} dict_t;

void print_error_and_exit(char*, ...);

byte_t * pack_couple(int , int);
int* unpack_couple(const byte_t*);

void emit_code(int, int);
int* receive_couple_of_code(int);

void empty_dictionary(dict_t*);
int dict_contains(dict_t*, char*);
void add_entry(dict_t*, char*);
char * get_entry(dict_t *, int);

#endif