#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "common/util.h"
#include "record.h"

/* Expected argv arguments, in that order:
 * datafile, rangeStart, rangeEnd, searchPattern */
int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "[Searcher] Invalid number of arguments.\n");
        return EC_ARG;
    }
    char *datafile = argv[1];
    char *strtolEndptr = NULL;
    int rangeStart = (int) strtol(argv[2], &strtolEndptr, 10);
    if (*strtolEndptr != '\0' || rangeStart < 0) {
        fprintf(stderr, "[Searcher] Invalid arguments: rangeStart must be a non-negative integer.\n");
        return EC_ARG;
    }
    int rangeEnd = (int) strtol(argv[3], &strtolEndptr, 10);
    if (*strtolEndptr != '\0' || rangeEnd < rangeStart) {
        fprintf(stderr, "[Searcher] Invalid arguments: rangeEnd must be an integer greater than or equal to rangeStart.\n");
        return EC_ARG;
    }
    char *pattern = argv[4];

    FILE *datafp = fopen(datafile, "rb");
    if (datafp == NULL) {
        perror("[Searcher] fopen");
        return EC_FILE;
    }
    struct stat st;
    if (stat(datafile, &st) != 0) {     // file may not exist, be inaccessible, etc
        perror("[Searcher] stat");
        return EC_FILE;
    }
    if (st.st_size < (rangeEnd + 1) * sizeof(Record)) {
        fprintf(stderr, "[Searcher] rangeEnd surpasses file end.\n");
        return EC_INVALID;
    }
    fseek(datafp, rangeStart * sizeof(Record), SEEK_SET);       // move file position to rangeStart Record

    Record currRecord;
    for (int i = rangeStart; i <= rangeEnd; i++) {
        fread(&currRecord, sizeof(struct record), 1, datafp);
        printRecord(currRecord);
    }

    fclose(datafp);
    return EC_OK;
}
