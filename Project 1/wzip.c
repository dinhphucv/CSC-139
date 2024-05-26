#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("wzip: file1 [file2 ...]\n");
        return 1;
    }

    FILE *fptr;

    char *aString = NULL;
    size_t bufferSize = 0;

    char previousCharacter;
    int characterCount = 0;

    for (int a = 1; a < argc; a++) {

        fptr = fopen(argv[a], "r");

        while (getline(&aString, &bufferSize, fptr) != -1) {

            for (int b = 0; b < strlen(aString); b++) {

                if (b == 0 && characterCount == 0) {

                    previousCharacter = aString[0];
                    characterCount = 1;
                    continue;

                }

                if (aString[b] == previousCharacter) {

                    characterCount++;

                } else {

                    fwrite(&characterCount, 4, 1, stdout);
                    fwrite(&previousCharacter, 1, 1, stdout);
                    previousCharacter = aString[b];
                    characterCount = 1;

                }

            }

        }

        fclose(fptr);

    }

    fwrite(&characterCount, 4, 1, stdout);
    fwrite(&previousCharacter, 1, 1, stdout);

}
