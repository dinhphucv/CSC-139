#include <stdio.h>

int main(int argc, char *argv[]) {
    FILE *fptr;

    for (int i = 1; i < argc; i++) {

        fptr = fopen(argv[i], "r");

        if (fptr != NULL) {

            char aString[100];

            while (fgets(aString, 100, fptr) != NULL) {
                printf("%s", aString);
            }

            fclose(fptr);

        } else {

            printf("wcat: cannot open file\n");
            return 1;

        }

    }
    
}
