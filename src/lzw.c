#include "lzw.h"
#include "utils.h"

void encode_text(char * filename, int dest) {
    printf("Start encode_text\n");
    FILE *fd = fopen(filename, "r");

    if (fd == NULL) {
        print_error_and_exit("Could not open file %s\n", filename);
    }

    dict_t* d = malloc(sizeof(dict_t));

    int codes_emitted = 0;
    char next_char = (char) fgetc(fd);
    char *str_next_char = malloc(2);
    str_next_char[0] = next_char;
    str_next_char[1] = '\0';

    char *str_current_symbol = malloc(1);
    *str_current_symbol = '\0';

    while (next_char != EOF) {
        char *str_eval = malloc(strlen(str_current_symbol) + 1);
        strcpy(str_eval, str_current_symbol);
        strcat(str_eval, str_next_char);

        int index = dict_contains(d, str_eval);

        if (index != -1) {
            str_current_symbol = str_eval;
        }
        else {
            emit_code(dest, dict_contains(d, str_current_symbol));
            codes_emitted++;

            if (d->nb_entries >= MAX_SIZE_DICTIONARY) {
                empty_dictionary(d);
                emit_code(dest, EMPTY_COMMAND);
                codes_emitted++;
            }

            add_entry(d, str_eval);
            strcpy(str_current_symbol, str_next_char);
        }

        next_char = (char) fgetc(fd);
        str_next_char[0] = next_char;
    }

    emit_code(dest, END_OF_STREAM);

    if (codes_emitted % 2 == 0) {
        emit_code(dest, END_OF_STREAM);
    }

    if (fclose(fd) == -1) {
        print_error_and_exit("Could not close %s", filename);
    }

    if (close(dest) == -1) {
        print_error_and_exit("Could not close destination");
    }

    printf("End encode_text\n");
}

void decode_text(int src, FILE * dest) {

    dict_t* d = malloc(sizeof(dict_t));

    int *couple = receive_couple_of_code(src);
    int k = couple[0];
    fprintf(dest, "%c", (char) k);

    char *w = (char *)malloc(2);
    w[0] = (char) k; // On sait que le premier code est forcement < 256
    w[1] = '\0';

    int sel = 1;

    while(couple[0] != END_OF_STREAM && couple[1] != END_OF_STREAM) {
        k = couple[sel];

        if (k == EMPTY_COMMAND) {
            empty_dictionary(d);
        } else if (k < d->nb_entries + 258) {
            char *entry = get_entry(d, k);
            fprintf(dest, "%s", entry);

            char *new_entry = malloc(strlen(w) + 2);
            strncpy(new_entry, w, strlen(w));
            new_entry[strlen(w)] = entry[0];

            add_entry(d, new_entry);
            w = entry;
        } else {
            char *new_entry = malloc(strlen(w) + 2);
            strncpy(new_entry, w, strlen(w));
            new_entry[strlen(w)] = w[0];

            fprintf(dest, "%s", new_entry);

            add_entry(d, new_entry);
            w = new_entry;
        }

        if (sel == 1)	{										//le couple est entièrement traité, on peut en récupérer un nouveau
            couple = receive_couple_of_code(src);
            sel = 0;
        }
        else {
            sel = 1;
        }
    }

    if(couple[0] != END_OF_STREAM) {
        k = couple[0];
        char * entry = get_entry(d, k);
        fprintf(dest, "%s", entry);
    }

    if(close(src) == -1) {
        print_error_and_exit("Error while closing source");
    }

    printf("\nEnd of decoder\n");
}