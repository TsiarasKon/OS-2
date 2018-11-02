#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "util.h"

int main(int argc, char *argv[]) {
    const char argErrorMsg[] = "Invalid arguments. Please run \"$ ./myfind -h Height -d Datafile -p Pattern [-s]\"\n";
    if (argc != 7 && argc != 8) {
        printf("%s", argErrorMsg);
        return EC_ARG;
    }
    int height = -1;
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
                    free(datafile);
                    free(pattern);
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
                free(datafile);
                free(pattern);
                return EC_ARG;
        }
    }
    if (height == -1 || datafile == NULL || pattern == NULL) {
        printf("%s", argErrorMsg);
        free(datafile);
        free(pattern);
        return EC_ARG;
    }

    printf("%d %s %s %d\n", height, datafile, pattern, skew);

    free(datafile);
    free(pattern);
    return 0;
}