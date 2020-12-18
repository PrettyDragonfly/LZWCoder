#ifndef LZW_H
#define LZW_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define EMPTY_COMMAND 256
#define END_OF_STREAM 257

void encode_text(char *, int);
void decode_text(int, FILE *);

#endif