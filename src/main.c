#include "lzw.h"
#include "utils.h"
#include <sys/wait.h>

void parent(char * filename, int pipe) {
    encode_text(filename, pipe);

    if (wait(0) == -1) {
        print_error_and_exit("Error while waiting");
    }
}

void child(int pipe, FILE * dest) {
    decode_text(pipe, dest);
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        print_error_and_exit("Please provide the filename as an argument");
    }

    int pfd[2];
    if(pipe(pfd) == -1) {
        print_error_and_exit("Error while opening pipe");
    }

    int pid = fork();
    if (pid == 0) {
        child(pfd[0], stdout);
        exit(0);
    }

    parent(argv[0], pfd[1]);

    return 0;
}
