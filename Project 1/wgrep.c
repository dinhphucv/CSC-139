#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("wgrep: searchterm [file ...]\n");
        return 1;
    }

    FILE *fptr;

    if (argc == 3) {

        fptr = fopen(argv[2], "r");

        if (fptr == NULL) {

            printf("wgrep: cannot open file\n");
            return 1;

        }

    } else if (argc == 2) {

        fptr = stdin;

    }

    char *aString = NULL;
    size_t bufferSize = 0;

    while (getline(&aString, &bufferSize, fptr) != -1) {

        if (strstr(aString, argv[1]) != NULL) {
            printf("%s", aString);
        }

    }

    if (fptr != stdin) {
        fclose(fptr);
    }

}
