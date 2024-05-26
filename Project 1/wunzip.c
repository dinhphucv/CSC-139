#include <stdio.h>

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("wunzip: file1 [file2 ...]\n");
        return 1;
    }

    FILE *fptr;

    int characterCount;
    char character;

    for (int i = 1; i < argc; i++) {

        fptr = fopen(argv[i], "r");

        while (fread(&characterCount, 4, 1, fptr) == 1 && fread(&character, 1, 1, fptr)) {

            for (int c = 1; c <= characterCount; c++) {
                printf("%c", character);
            }

        }

        fclose(fptr);

    }

}
