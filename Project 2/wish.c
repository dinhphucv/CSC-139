#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_ARGS 5
#define MAX_PARALLEL_COMMANDS 10
#define MAX_COMMANDS_SAVED 100

void printErrorMessage();
void printPrompt();
void saveCommand(char *commandToSave);
void clearCommandsSaved();
void printCommandsSaved();

char *commandsSaved[MAX_COMMANDS_SAVED] = {NULL};
unsigned int numberOfCommandsSaved = 0;

int main(int argc, char *argv[]) {

    FILE *fptr;
    int batchMode = 0;
    int historyMode = 0;
    char *historyCommandToRun = strdup("");

    if (argc > 2) {
        printErrorMessage();
        exit(1);
    } else if (argc == 2) {
        fptr = fopen(argv[1], "r");
        if (fptr == NULL) {
            printErrorMessage();
            exit(1);
        }
        batchMode = 1;
    } else {
        fptr = stdin;
    }

    char *searchPath = strdup("/bin");

    char *aLine;
    size_t aLineSize;
    ssize_t nCharactersRead;

    while (1) {

        if ((!batchMode) && (!historyMode)) {
            printPrompt();
        }

        if (!historyMode) {
            nCharactersRead = getline(&aLine, &aLineSize, fptr);

            if (nCharactersRead == -1) {
                exit(0);
            }

            aLine[strcspn(aLine, "\n")] = '\0';

            if (strcmp(aLine, "") != 0) {
                saveCommand(aLine);
            } else {
                continue;
            }
        } else {
            aLine = strdup(historyCommandToRun);
            historyMode = 0;
        }

        char *aLineBackup = strdup(aLine);

        char *commands[MAX_PARALLEL_COMMANDS] = {NULL};
        int numberOfCommands = 0;

        if (strsep(&aLine, "&") != NULL) {
            aLine = strdup(aLineBackup);
            int commandIndex = -1;
            while (aLine != NULL) {
                commandIndex++;
                commands[commandIndex] = strsep(&aLine, "&");
            }
            numberOfCommands = commandIndex + 1;
        }

        free(aLineBackup);

        for (int commandIndex = 0; commandIndex < numberOfCommands; commandIndex++) {
            char *command = strdup(commands[commandIndex]);

            char *outputFile = NULL;
            int redirectionMode = 0;

            if (strchr(command, '>') != NULL) {
                if (command[strlen(command) -1] == '>') {
                    printErrorMessage();
                    continue;
                }

                char *redirectionArgv[MAX_ARGS] = {NULL};
                int redirectionArgvIndex = -1;
                while (command != NULL) {
                    redirectionArgvIndex++;
                    redirectionArgv[redirectionArgvIndex] = strsep(&command, ">");
                }

                int redirectionArgc = redirectionArgvIndex + 1;

                if (redirectionArgc > 2) {
                    printErrorMessage();
                    continue;
                }

                if (strchr(redirectionArgv[redirectionArgvIndex], ' ') != NULL) {
                    char *redirectionOutputArgv[MAX_ARGS] = {NULL};
                    int redirectionOutputArgvIndex = -1;
                    while (redirectionArgv[redirectionArgvIndex] != NULL) {
                        char *temporaryArgv = strsep(&redirectionArgv[redirectionArgvIndex], " ");
                        if (strcmp(temporaryArgv, "") != 0) {
                            redirectionOutputArgvIndex++;
                            redirectionOutputArgv[redirectionOutputArgvIndex] = temporaryArgv;
                        }
                    }

                    if (redirectionOutputArgvIndex != 0) {
                        printErrorMessage();
                        continue;
                    } else {
                        redirectionMode = 1;
                        outputFile = redirectionOutputArgv[0];
                        command = strdup(redirectionArgv[redirectionArgvIndex - 1]);
                        if (strcmp(command, "") == 0) {
                            printErrorMessage();
                            continue;
                        }
                    }

                } else {
                    redirectionMode = 1;
                    outputFile = redirectionArgv[redirectionArgvIndex];
                    command = strdup(redirectionArgv[redirectionArgvIndex - 1]);
                }
            }

            char *newArgv[MAX_ARGS] = {NULL};
            int newArgvIndex = -1;

            while (command != NULL) {
                char *temporaryArgv = strsep(&command, " ");
                if (strcmp(temporaryArgv, "") != 0) {
                    newArgvIndex++;
                    newArgv[newArgvIndex] = temporaryArgv;
                }
            }

            if (newArgvIndex == -1) {
                continue;
            }

            int newArgc = newArgvIndex + 1;

            char commandFirstCharacter = newArgv[0][0];

            if (strcmp(newArgv[0], "exit") == 0) {
                if (newArgc == 1) {
                    exit(0);
                } else {
                    printErrorMessage();
                }
            } else if (strcmp(newArgv[0], "cd") == 0) {
                if (newArgc != 2) {
                    printErrorMessage();
                } else {
                    chdir(newArgv[1]);
                }
            } else if (strcmp(newArgv[0], "path") == 0) {
                free(searchPath);
                searchPath = strdup("");
                if (newArgc > 1) {
                    char *newSearchPath;
                    for (int i = 1; i < newArgc; i++) {
                        char firstCharacter = newArgv[i][0];
                        if (firstCharacter == '/') {
                            newSearchPath = malloc(strlen(searchPath) + strlen(newArgv[i]) + 1);
                            strcpy(newSearchPath, searchPath);
                            strcat(newSearchPath, ":");
                            strcat(newSearchPath, newArgv[i]);
                            free(searchPath);
                            searchPath = newSearchPath;
                        } else {
                            char *currentWorkingDirectory = getcwd(NULL, 0);
                            char *newPath = malloc(strlen(currentWorkingDirectory) + strlen(newArgv[i]) + 1);
                            strcpy(newPath, currentWorkingDirectory);
                            strcat(newPath, "/");
                            strcat(newPath, newArgv[i]);

                            newSearchPath = malloc(strlen(searchPath) + strlen(newPath) + 1);
                            strcpy(newSearchPath, searchPath);
                            strcat(newSearchPath, ":");
                            strcat(newSearchPath, newPath);

                            free(searchPath);
                            searchPath = newSearchPath;
                            free(currentWorkingDirectory);
                            free(newPath);
                        }
                    }
                }
            } else if (strcmp(newArgv[0], "history") == 0) {
                if (newArgc == 1) {
                    printCommandsSaved();
                } else {
                    printErrorMessage();
                }
            } else if (strcmp(newArgv[0], "clearHistory") == 0) {
                if (newArgc == 1) {
                    clearCommandsSaved();
                } else {
                    printErrorMessage();
                }
            } else if (strcmp(newArgv[0], "cat") == 0) {
                if (newArgc != 1) {
                    FILE *tempFptr;
                    char *tempALine;
                    size_t tempALineSize;
                    int originalStdout = dup(1);
                    if (redirectionMode) {
                        int fd = open(outputFile, O_CREAT | O_WRONLY | O_TRUNC, 0644);
                        dup2(fd, 1);
                    }
                    for (int i = 1; i < newArgc; i++) {
                        tempFptr = fopen(newArgv[i], "r");
                        if (tempFptr != NULL) {
                            while (getline(&tempALine, &tempALineSize, tempFptr) != -1) {
                                printf("%s", tempALine);
                            }
                            fclose(tempFptr);
                        } else {
                            printErrorMessage();
                            continue;
                        }
                    }
                    if (redirectionMode) {
                        dup2(originalStdout, 1);
                        close(originalStdout);
                    }
                } else {
                    printErrorMessage();
                    continue;
                }
            } else if (commandFirstCharacter == '!') {
                if (newArgc == 1) {
                    char *tempCommand = strdup(newArgv[0]);
                    char *temp = tempCommand + 1;
                    int number = atoi(temp);
                    if ((number != 0) && (number < numberOfCommandsSaved)) {
                        free(historyCommandToRun);
                        historyCommandToRun = strdup(commandsSaved[number - 1]);
                        free(commandsSaved[numberOfCommandsSaved - 1]);
                        commandsSaved[numberOfCommandsSaved - 1] = strdup(historyCommandToRun);
                        historyMode = 1;
                        break;
                    } else {
                        printErrorMessage();
                        break;
                    }
                } else {
                    printErrorMessage();
                    break;
                }
            } else {
                pid_t pid = fork();

                if (pid < 0) {

                } else if (pid == 0) {

                    char *programPath;

                    while (searchPath != NULL) {
                        char *temporaryPath = strsep(&searchPath, ":");
                        if (strcmp(temporaryPath, "") != 0) {
                            programPath = malloc(strlen(temporaryPath) + strlen(newArgv[0]) + 1);
                            strcpy(programPath, temporaryPath);
                            strcat(programPath, "/");
                            strcat(programPath, newArgv[0]);

                            if (access(programPath, X_OK) == 0) {
                                break;
                            }
                        }
                    }

                    if (redirectionMode) {
                        int fd = open(outputFile, O_CREAT | O_WRONLY | O_TRUNC, 0644);
                        dup2(fd, 1);
                    }
                    if (execv(programPath, newArgv) == -1) {
                        printErrorMessage();
                        exit(127);
                    }
                }
            }
        }

        while (wait(NULL) > 0);

    }
}

void printErrorMessage() {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

void printPrompt() {
    printf("wish> ");
}

void saveCommand(char *commandToSave) {
    if (numberOfCommandsSaved < MAX_COMMANDS_SAVED) {
        commandsSaved[numberOfCommandsSaved] = strdup(commandToSave);
        numberOfCommandsSaved++;
    } else {
        clearCommandsSaved();
    }
}

void clearCommandsSaved() {
    for (int i = 0; i < MAX_COMMANDS_SAVED; i++) {
        free(commandsSaved[i]);
        commandsSaved[i] = NULL;
    }
    numberOfCommandsSaved = 0;
}

void printCommandsSaved() {
    for (int i = 0; i < numberOfCommandsSaved; i++) {
        printf("%d %s\n", i + 1, commandsSaved[i]);
    }
}
