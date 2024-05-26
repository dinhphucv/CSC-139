//
// Created by dinhphucv on 4/30/24.
//

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define KILOBYTE 1024
#define MEGABYTE (1024 * 1024)
#define GIGABYTE (1024 * 1024 * 1024)

#define MAX_FILES 10

void printHelp();
void printInodeInfo(char *file);
void printDirectoryInfo(char *directory);

int recursive = 0;
int humanReadable = 0;
int jsonFormat = 0;

int first = 1;

int main(int argc, char *argv[]) {

    char *files[MAX_FILES] = {NULL};
    int numberOfFiles = 0;
    char *directory = NULL;
    char *logFile = NULL;

    if (argc == 1) {
        printHelp();
        return 0;
    }

    for (int i = 1; i < argc; i++) {

        if ((strcmp(argv[i], "-?") == 0) || (strcmp(argv[i], "--help") == 0)) {

            printHelp();
            return 0;

        } else if ((strcmp(argv[i], "-i") == 0) || (strcmp(argv[i], "--inode") == 0)) {

            while (i + 1 < argc && argv[i + 1][0] != '-') {
                files[numberOfFiles] = argv[i + 1];
                numberOfFiles++;
                i++;
            }

        } else if ((strcmp(argv[i], "-a") == 0) || (strcmp(argv[i], "--all") == 0)) {

            directory = ".";

            if (i + 1 < argc) {

                if (argv[i + 1][0] != '-') {

                    directory = argv[i + 1];
                    i++;

                    if (i + 1 < argc && (strcmp(argv[i + 1], "-r") == 0 || strcmp(argv[i + 1], "--recursive") == 0)) {
                        recursive = 1;
                    }

                } else if (strcmp(argv[i + 1], "-r") == 0 || strcmp(argv[i + 1], "--recursive") == 0) {
                    recursive = 1;
                    i++;
                }

            }

        } else if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--human") == 0)) {
            humanReadable = 1;
        } else if ((strcmp(argv[i], "-f") == 0) || (strcmp(argv[i], "--format") == 0)) {

            if (i + 1 < argc) {

                if (strcmp(argv[i + 1], "json") == 0) {
                    jsonFormat = 1;
                    i++;
                } else if (strcmp(argv[i + 1], "text") == 0) {
                    i++;
                }

            }

        } else if ((strcmp(argv[i], "-l") == 0) || (strcmp(argv[i], "--log") == 0)) {

            if (i + 1 < argc) {

                if (argv[i + 1][0] != '-') {
                    logFile = argv[i + 1];
                    i++;
                }

            }

        } else {

            if (argv[i][0] != '-') {

                files[numberOfFiles] = argv[i];
                numberOfFiles++;

                while (i + 1 < argc && argv[i + 1][0] != '-') {

                    files[numberOfFiles] = argv[i + 1];
                    numberOfFiles++;
                    i++;

                }

            }

        }

    }

    int originalStdout = dup(1);
    if (logFile != NULL) {

        int fd = open(logFile, O_CREAT | O_WRONLY | O_TRUNC, 0644);

        if (fd == -1) {
            fprintf(stderr, "Error opening log file for %s: %s\n", logFile, strerror(errno));
            return 1;
        }

        dup2(fd, 1);
        close(fd);

    }

    if (numberOfFiles != 0 && directory == NULL) {

        if (jsonFormat) {
            printf("[");
        }

        for (int i = 0; i < numberOfFiles; i++) {
            printInodeInfo(files[i]);
        }

        if (jsonFormat) {
            printf("]\n");
        }

    } else if (numberOfFiles == 0 && directory != NULL) {

        if (jsonFormat) {
            printf("[");
        }

        printDirectoryInfo(directory);

        if (jsonFormat) {
            printf("]\n");
        }

    } else {

        fprintf(stderr, "Incorrect usage. For proper usage, please run with \"-?\" option\n");

    }

    if (logFile != NULL) {
        fflush(stdout);
        dup2(originalStdout, 1);
    }

    close(originalStdout);

}

void printHelp() {
    printf("Usage: inspect [OPTION]... [FILE]...\n");
    printf("Options:\n");
    printf("  -?, --help\t\t\tdisplay this help and exit\n");
    printf("  -i, --inode <file_path>\tdisplay detailed inode information for the specified file\n");
    printf("  -a, --all [directory_path]\tdisplay inode information for all files within the specified directory\n");
    printf("      -r, --recursive\t\toptional flag, require -a or --all recursive listing\n");
    printf("  -h, --human\t\t\toutput all sizes in kilobytes (K), megabytes (M), or gigabytes (G) and all dates in a human-readable form\n");
    printf("  -f, --format [text|json]\tspecify the output format\n");
    printf("  -l, --log <log_file>\t\tlog operations to a specified file\n");
}

void printInodeInfo(char *file) {

    struct stat fileInfo;

    if (stat(file, &fileInfo) != 0) {
        fprintf(stderr, "Error getting file info for %s: %s\n", file, strerror(errno));
        return;
    }

    if (first) {
        first = 0;
    } else {

        if (jsonFormat) {
            printf(",");
        }
        printf("\n");

    }

    char readableAccessTime[20], readableModificationTime[20], readableStatusChangeTime[20];

    if (humanReadable) {
        strftime(readableAccessTime, 20, "%Y-%m-%d %H:%M:%S", localtime(&fileInfo.st_atime));
        strftime(readableModificationTime, 20, "%Y-%m-%d %H:%M:%S", localtime(&fileInfo.st_mtime));
        strftime(readableStatusChangeTime, 20, "%Y-%m-%d %H:%M:%S", localtime(&fileInfo.st_ctime));
    }

    if (jsonFormat) {
        printf("{\n");
        printf("  \"filePath\": \"%s\",\n", file);
        printf("  \"inode\": {\n");
        printf("    \"number\": %lu,\n", fileInfo.st_ino);
        printf("    \"type\": ");
        // Starting of type
        if (S_ISREG(fileInfo.st_mode))
            printf("\"regular file\",\n");
        else if (S_ISDIR(fileInfo.st_mode))
            printf("\"directory\",\n");
        else if (S_ISCHR(fileInfo.st_mode))
            printf("\"character device\",\n");
        else if (S_ISBLK(fileInfo.st_mode))
            printf("\"block device\",\n");
        else if (S_ISFIFO(fileInfo.st_mode))
            printf("\"FIFO (named pipe)\",\n");
        else if (S_ISLNK(fileInfo.st_mode))
            printf("\"symbolic link\",\n");
        else if (S_ISSOCK(fileInfo.st_mode))
            printf("\"socket\",\n");
        else
            printf("\"unknown?\",\n");
        // Ending of type
        printf("    \"permissions\": ");
        printf("\"");
        // Starting of permission
        printf((fileInfo.st_mode & S_IRUSR) ? "r" : "-");
        printf((fileInfo.st_mode & S_IWUSR) ? "w" : "-");
        printf((fileInfo.st_mode & S_IXUSR) ? "x" : "-");

        printf((fileInfo.st_mode & S_IRGRP) ? "r" : "-");
        printf((fileInfo.st_mode & S_IWGRP) ? "w" : "-");
        printf((fileInfo.st_mode & S_IXGRP) ? "x" : "-");

        printf((fileInfo.st_mode & S_IROTH) ? "r" : "-");
        printf((fileInfo.st_mode & S_IWOTH) ? "w" : "-");
        printf((fileInfo.st_mode & S_IXOTH) ? "x" : "-");
        // Ending of permission
        printf("\",\n");
        printf("    \"linkCount\": %lu,\n", fileInfo.st_nlink);
        printf("    \"uid\": %u,\n", fileInfo.st_uid);
        printf("    \"gid\": %u,\n", fileInfo.st_gid);
        if (humanReadable) {
            printf("    \"size\": ");
            unsigned long newSize;
            if (fileInfo.st_size >= GIGABYTE) {
                newSize = fileInfo.st_size / GIGABYTE;
                if ((fileInfo.st_size % GIGABYTE) != 0) {
                    newSize++;
                }
                printf("\"%luG\",\n", newSize);
            } else if (fileInfo.st_size >= MEGABYTE) {
                newSize = fileInfo.st_size / MEGABYTE;
                if ((fileInfo.st_size % MEGABYTE) != 0) {
                    newSize++;
                }
                printf("\"%luM\",\n", newSize);
            } else if (fileInfo.st_size >= KILOBYTE) {
                newSize = fileInfo.st_size / KILOBYTE;
                if ((fileInfo.st_size % KILOBYTE) != 0) {
                    newSize++;
                }
                printf("\"%luK\",\n", newSize);
            } else {
                printf("\"%luB\",\n", fileInfo.st_size);
            }
            printf("    \"accessTime\": \"%s\",\n", readableAccessTime);
            printf("    \"modificationTime\": \"%s\",\n", readableModificationTime);
            printf("    \"statusChangeTime\": \"%s\"\n", readableStatusChangeTime);
        } else {
            printf("    \"size\": %lu,\n", fileInfo.st_size);
            printf("    \"accessTime\": %ld,\n", fileInfo.st_atime);
            printf("    \"modificationTime\": %ld,\n", fileInfo.st_mtime);
            printf("    \"statusChangeTime\": %ld\n", fileInfo.st_ctime);
        }
        printf("  }\n");
        printf("}");
    } else {
        printf("Information for %s:\n", file);
        printf("File Inode: %lu\n", fileInfo.st_ino);
        printf("File Type: ");
        if (S_ISREG(fileInfo.st_mode))
            printf("regular file\n");
        else if (S_ISDIR(fileInfo.st_mode))
            printf("directory\n");
        else if (S_ISCHR(fileInfo.st_mode))
            printf("character device\n");
        else if (S_ISBLK(fileInfo.st_mode))
            printf("block device\n");
        else if (S_ISFIFO(fileInfo.st_mode))
            printf("FIFO (named pipe)\n");
        else if (S_ISLNK(fileInfo.st_mode))
            printf("symbolic link\n");
        else if (S_ISSOCK(fileInfo.st_mode))
            printf("socket\n");
        else
            printf("unknown?\n");

        printf("Permission: ");
        printf((fileInfo.st_mode & S_IRUSR) ? "r" : "-");
        printf((fileInfo.st_mode & S_IWUSR) ? "w" : "-");
        printf((fileInfo.st_mode & S_IXUSR) ? "x" : "-");

        printf((fileInfo.st_mode & S_IRGRP) ? "r" : "-");
        printf((fileInfo.st_mode & S_IWGRP) ? "w" : "-");
        printf((fileInfo.st_mode & S_IXGRP) ? "x" : "-");

        printf((fileInfo.st_mode & S_IROTH) ? "r" : "-");
        printf((fileInfo.st_mode & S_IWOTH) ? "w" : "-");
        printf((fileInfo.st_mode & S_IXOTH) ? "x" : "-");
        printf("\n");
        printf("Number of Hard Links: %lu\n", fileInfo.st_nlink);
        printf("UID: %u\n", fileInfo.st_uid);
        printf("GID: %u\n", fileInfo.st_gid);
        if (humanReadable) {
            printf("File Size: ");
            unsigned long newSize;
            if (fileInfo.st_size >= GIGABYTE) {
                newSize = fileInfo.st_size / GIGABYTE;
                if ((fileInfo.st_size % GIGABYTE) != 0) {
                    newSize++;
                }
                printf("%luG\n", newSize);
            } else if (fileInfo.st_size >= MEGABYTE) {
                newSize = fileInfo.st_size / MEGABYTE;
                if ((fileInfo.st_size % MEGABYTE) != 0) {
                    newSize++;
                }
                printf("%luM\n", newSize);
            } else if (fileInfo.st_size >= KILOBYTE) {
                newSize = fileInfo.st_size / KILOBYTE;
                if ((fileInfo.st_size % KILOBYTE) != 0) {
                    newSize++;
                }
                printf("%luK\n", newSize);
            } else {
                printf("%luB\n", fileInfo.st_size);
            }
            printf("Last Access Time: %s\n", readableAccessTime);
            printf("Last Modification Time: %s\n", readableModificationTime);
            printf("Last Status Change Time: %s\n", readableStatusChangeTime);
        } else {
            printf("File Size: %lu bytes\n", fileInfo.st_size);
            printf("Last Access Time: %ld\n", fileInfo.st_atime);
            printf("Last Modification Time: %ld\n", fileInfo.st_mtime);
            printf("Last Status Change Time: %ld\n", fileInfo.st_ctime);
        }
    }

}

void printDirectoryInfo(char *directory) {

    DIR *dp = opendir(directory);

    if (dp == NULL) {
        fprintf(stderr, "Error getting directory info for %s: %s\n", directory, strerror(errno));
        return;
    }

    struct dirent *d;

    while ((d = readdir(dp)) != NULL) {

        if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0) {
            continue;
        }

        char *path = malloc(strlen(directory) + strlen(d->d_name) + 2);
        strcpy(path, directory);
        if (directory[strlen(directory) - 1] != '/' ) {
            strcat(path, "/");
        }
        strcat(path, d->d_name);

        printInodeInfo(path);

        if (recursive) {

            if (d->d_type == DT_DIR) {
                printDirectoryInfo(path);
            }

        }

        free(path);

    }

    closedir(dp);

}