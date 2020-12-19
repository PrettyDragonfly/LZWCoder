#include "lzw.h"
#include "utils.h"
#include <sys/wait.h>

void parent(char * filename, int pipe) {
    printf("Start parent\n");
    encode_text(filename, pipe);

    if (wait(NULL) == -1) {
        print_error_and_exit("Error while waiting");
    }
    printf("End parent\n");
}

void child(int pipe, FILE * dest) {
    printf("Start child\n");
    decode_text(pipe, dest);
    printf("End child\n");
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        print_error_and_exit("Please provide the filename as an argument");
    }

    int pfd[2];
    if(pipe(pfd) == -1) {
        print_error_and_exit("Error while opening pipe");
    }

    FILE *output_file = fopen("resultat.txt", "w+");
    if (output_file == NULL) {
        print_error_and_exit("Error while opening output file");
    }

    int pid = fork();
    if (pid == 0) {
        child(pfd[0], output_file);
        exit(0);
    }

    parent(argv[1], pfd[1]);

    return 0;
}
