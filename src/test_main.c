#include "utils.h"
#include "lzw.h"

int test_pack_unpack_couple() {
    for (int i = 0; i < 4096; i++) {
        for (int j = 0; j < 4096; j++) {
            int *couple = unpack_couple(pack_couple(i, j));
            if (couple[0] !=  i || couple[1] != j) {
                printf("Test failed: expected: (%d, %d) but received (%d, %d)\n", i, j, couple[0], couple[1]);
                return -1;
            }
        }
    }
    return 0;
}

void test_emit_receive() {
    int pfd[2];
    if(pipe(pfd) == -1) {
        print_error_and_exit("Error while opening pipe");
    }

    for (int i = 0; i < 10; i++) {
        emit_code(pfd[1], i);
    }
    emit_code(pfd[1], 0);
    emit_code(pfd[1], 0);

    int * couple = receive_couple_of_code(pfd[0]);
    while(couple[0] != 0 || couple[1] != 0) {
        couple = receive_couple_of_code(pfd[0]);
    }

    if (close(pfd[0]) == -1) {
        print_error_and_exit("Error while closing pipe");
    }

    if (close(pfd[1]) == -1) {
        print_error_and_exit("Error while closing pipe");
    }
}

int test_add_dictionary() {
    dict_t* d = malloc(sizeof(dict_t));
    char *text = "Hello world";
    add_entry(d, text);

    if(strcmp(d->entries[0], text) != 0) {
        printf("Test failed: d->entries[0] is not 'Hello world'\n");
        return -1;
    }

    return 0;
}

int test_add_dictionary_size() {
    dict_t* d = malloc(sizeof(dict_t));
    char *text = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
    add_entry(d, text);

    if(strcmp(d->entries[0], text) != 0) {
        printf("Test failed: d->entries[0] is not '%s'\n", text);
        return -1;
    }

    return 0;
}

int test_contain_dictionary_true() {
    dict_t* d = malloc(sizeof(dict_t));
    char *text = "Hello world";
    add_entry(d, text);

    int position = dict_contains(d, text);
    if(position != 258) {
        printf("Test failed: expected: %d but received %d\n", 258, position);
        return -1;
    }

    return 0;
}

int test_contain_dictionary_false() {
    dict_t* d = malloc(sizeof(dict_t));
    char *text = "Hello world";
    char *text2 = "Hello";
    add_entry(d, text);

    int position = dict_contains(d, text2);
    if(position != -1) {
        printf("Test failed: expected: %d but received %d\n", -1, position);
        return -1;
    }

    return 0;
}

int test_contain_dictionary_ascii() {
    dict_t* d = malloc(sizeof(dict_t));
    char *text = "a";

    int position = dict_contains(d, text);
    if(position != 97) {
        printf("Test failed: expected: %d but received %d\n", 97, position);
        return -1;
    }

    return 0;
}

int test_empty_dictionary() {
    dict_t* d = malloc(sizeof(dict_t));
    char *text = "Hello world";
    add_entry(d, text);
    empty_dictionary(d);

    int position = dict_contains(d, text);
    if(position != -1) {
        printf("Test failed: expected: %d but received %d\n", -1, position);
        return -1;
    }

    return 0;
}

int test_get_entry_dict() {
    dict_t* d = malloc(sizeof(dict_t));
    char *text = "Hello world";
    char *text2 = "a";
    add_entry(d, text);

    char * entry = get_entry(d, 258);
    if(strcmp(text, entry) != 0) {
        printf("Test failed: expected: %s but received %s\n", text, entry);
        return -1;
    }

    entry = get_entry(d, 97);
    if(strcmp(text2, entry) != 0) {
        printf("Test failed: expected: %s but received %s\n", text2, entry);
        return -1;
    }

    return 0;
}

void test_encode(char *filename) {
    int pfd[2];
    if(pipe(pfd) == -1) {
        print_error_and_exit("Error while opening pipe");
    }

    encode_text(filename, pfd[1]);

    int * couple = receive_couple_of_code(pfd[0]);
    while(couple[0] != END_OF_STREAM && couple[1] != END_OF_STREAM) {
        couple = receive_couple_of_code(pfd[0]);
    }

    if(close(pfd[0]) == -1) {
        print_error_and_exit("Error while closing pipe");
    }
}

void test_decode(char * filename) {
    int pfd[2];
    if(pipe(pfd) == -1) {
        print_error_and_exit("Error while opening pipe");
    }

    encode_text(filename, pfd[1]);

    decode_text(pfd[0], stdout);
}

int main(int argc, char * argv[]) {
    char * filename = "files/base_a.txt";
    printf("test_pack_unpack_couple: %s\n", test_pack_unpack_couple() == 0 ? "SUCCESS" : "FAILED");
    test_emit_receive();

    printf("test_add_dictionary: %s\n", test_add_dictionary() == 0 ? "SUCCESS" : "FAILED");
    printf("test_add_dictionary_size: %s\n", test_add_dictionary_size() == 0 ? "SUCCESS" : "FAILED");
    printf("test_contain_dictionary_true: %s\n", test_contain_dictionary_true() == 0 ? "SUCCESS" : "FAILED");
    printf("test_contain_dictionary_false: %s\n", test_contain_dictionary_false() == 0 ? "SUCCESS" : "FAILED");
    printf("test_empty_dictionary: %s\n", test_empty_dictionary() == 0 ? "SUCCESS" : "FAILED");
    printf("test_contain_dictionary_ascii: %s\n", test_contain_dictionary_ascii() == 0 ? "SUCCESS" : "FAILED");
    printf("test_get_entry_dict: %s\n", test_get_entry_dict() == 0 ? "SUCCESS" : "FAILED");

    test_encode(filename);
    test_decode(filename);
}