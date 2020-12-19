#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Please provide the file to convert");
    }

    FILE *input_file = fopen(argv[1], "r");
    if (input_file == NULL) {
        fprintf(stderr, "Could not open %s\n", argv[1]);
    }

    char *extension = ".ascii";
    char *output_name = malloc(sizeof(char) * (strlen(argv[1]) + strlen(extension) + 1));
    strcpy(output_name, argv[1]);
    strcat(output_name, extension);

    FILE *output_file = fopen(output_name, "w+");
    if (output_file == NULL) {
        fprintf(stderr, "Could not open %s\n", output_name);
    }

    char c = (char) fgetc(input_file);
    while(c != EOF) {
        if(c > 0) {
            fprintf(output_file, "%c", c);
        }
        c = fgetc(input_file);
    }

    fclose(output_file);
    fclose(input_file);

    return 0;
}