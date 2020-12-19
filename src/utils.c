#include "utils.h"
//#define DEBUG

void print_error_and_exit(char *msg, ...) {
    va_list argptr;
    va_start(argptr, msg);
    fprintf(stderr, "\x1b[31m(╯°□°）╯︵ ┻━┻\x1b[0m: ");
    vfprintf(stderr, msg, argptr);
    fprintf(stderr, "\n");
    va_end(argptr);
    exit(1);
}

byte_t *pack_couple(int first, int second) {
    byte_t b1 = first >> 4;

    byte_t b2 = (first & 15) << 4;
    byte_t aux = second >> 8;
    b2 = b2 | aux;

    byte_t b3 = second & 255;

    byte_t *triplet = (byte_t *) calloc(3, sizeof(byte_t));
    triplet[0] = b1;
    triplet[1] = b2;
    triplet[2] = b3;

    return triplet;
}

int *unpack_couple(const byte_t *triplet) {
    int *couple = malloc(2 * sizeof(int));

    couple[0] = (triplet[0] << 0x04) | (triplet[1] >> 4u); // first 8 bits
    couple[1] = (triplet[1] & 0x0fu) << 8 | triplet[2];

    return couple;
}

void emit_code(int file, int code) {
    static int g_cpt = 0;
    static int cpt = 0;
    static int couple[2];

    if (code < 0) {
        print_error_and_exit("Non ASCII character detected, aborting");
    }

    cpt += 1;
    g_cpt++;
    if (cpt == 2) {
        cpt = 0;
        couple[1] = code;
        char *triplet = pack_couple(couple[0], couple[1]);

        if (write(file, triplet, 3 * sizeof(char)) == -1) {
            print_error_and_exit("Error while emitting (%d, %d)\n", couple[0], couple[1]);
        }

#ifdef DEBUG
        printf("Emitted couple (%d, %d) - %d\n", couple[0], couple[1], g_cpt);
#endif
    } else {
        couple[0] = code;
    }
}

int* receive_couple_of_code(int file) {
    char *triplet = (char *)calloc(3, sizeof(char));

    if (read(file, triplet, 3 * sizeof(char)) == -1)
        print_error_and_exit("Error while receiving couple of codes");

    int *couple = unpack_couple(triplet);

#ifdef DEBUG
    printf("Received couple (%d, %d)\n", couple[0], couple[1]);
#endif
    return couple;
}

void empty_dictionary(dict_t* d) {
#ifdef DEBUG
    printf("#### EMPTY DICTIONARY #####\n");
#endif
    d->nb_entries = 0;
}

int dict_contains(dict_t *d, char *str) {
    if (strlen(str) == 1) {
        return (char) str[0];
    }

    for (int i = 0; i < d->nb_entries; i++) {
        if (strcmp(str, d->entries[i]) == 0) {
            return i + 258;
        }
    }
    return -1;
}

void add_entry(dict_t* d, char* str) {
    d->entries[d->nb_entries] = malloc((strlen(str) + 1) * sizeof(char));
    strcpy(d->entries[d->nb_entries], str);
    d->nb_entries++;
}

char * get_entry(dict_t * d, int k) {
    if (k < 256) {
        char *s = (char *)malloc(1);
        *s = (char) k;
        return s;
    } else if (k < d->nb_entries + 258) {
        return d->entries[k - 258];
    }
    return NULL;
}