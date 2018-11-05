#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
#include "common/util.h"
#include "structs.h"

int main(int argc, char *argv[]) {
    const char argErrorMsg[] = "Invalid arguments. Please run \"$ ./myfind -h Height -d Datafile -p Pattern [-s]\"\n";
    if (argc != 7 && argc != 8) {
        printf("%s", argErrorMsg);
        return EC_ARG;
    }
    int height = 0;
    char *strtolEndptr = NULL;
    char *datafile = NULL;
    char *pattern = NULL;
    bool skew = false;
    int option;
    while ((option = getopt(argc, argv,"h:d:p:s")) != -1) {
        switch (option) {
            case 'h':
                height = (int) strtol(optarg, &strtolEndptr, 10);
                if (*strtolEndptr != '\0' || height < 1 || height > 5) {
                    printf("Invalid arguments: Height must be an integer between 1 and 5 (inclusive).\n");
                    free(datafile); free(pattern);
                    return EC_ARG;
                }
                break;
            case 'd':
                datafile = malloc(strlen(optarg) + 1);
                strcpy(datafile, optarg);
                break;
            case 'p':
                pattern = malloc(strlen(optarg) + 1);
                strcpy(pattern, optarg);
                break;
            case 's':
                skew = true;
                break;
            default:
                printf("%s", argErrorMsg);
                free(datafile); free(pattern);
                return EC_ARG;
        }
    }
    if (height == 0 || datafile == NULL || pattern == NULL) {
        printf("%s", argErrorMsg);
        free(datafile); free(pattern);
        return EC_ARG;
    }

    struct stat st;
    if (stat(datafile, &st) != 0) {     // file may not exist, be inaccessible, etc
        perror("stat");
        free(datafile); free(pattern);
        return EC_FILE;
    }
    if (st.st_size == 0) {
        fprintf(stderr, "Warning: Datafile is empty.\n");
    }
    int recordsNum = (int) (st.st_size / sizeof(Record));
    // Check if not whole division - recordsNum wouldn't have been an integer:
    if (recordsNum != (st.st_size / (double) sizeof(Record))) {
        fprintf(stderr, "Invalid Datafile format: Datafile's size is not a multiple of Record's size.");
        return EC_INVALID;
    }

    // TODO create pipe
    int smRootPid = fork();
    if (smRootPid < 0) {
        perror("fork");
        free(datafile); free(pattern);
        return EC_FORK;
    } else if (smRootPid == 0) {
        execl("../splitter_merger", "splitter_merger", datafile, 0, recordsNum - 1, pattern, height, skew, (char *) NULL);
        // this code will run only if exec fails:
        perror("execl");
        free(datafile); free(pattern);
        return EC_EXEC;
    }
    free(datafile);
    free(pattern);

    // TODO: stuff

    // TODO create pipe
    int sortPid = fork();
    if (sortPid < 0) {
        perror("fork");
        // TODO: frees?
        return EC_FORK;
    } else if (sortPid == 0) {
        execl("../sorter", "sorter", /* search results, */ (char *) NULL);
        perror("execl");
        return EC_EXEC;
    }

    return 0;
}