#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include "common/util.h"
#include "record.h"

/* Expected argv arguments, in that order:
 * datafile, rangeStart, rangeEnd, searchPattern[, rootPid] */
int main(int argc, char *argv[]) {
    clock_t start_t = clock();
    if (argc != 5 && argc != 6) {
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
    int rootPid;
    if (argc == 6) {
        rootPid = (int) strtol(argv[5], &strtolEndptr, 10);
        if (*strtolEndptr != '\0' || rootPid < 2) {
            fprintf(stderr, "[Searcher] Invalid arguments: invalid Root pid.\n");
            return EC_ARG;
        }
    } else {
        fprintf(stderr, "[Searcher] Warning: Root pid was not provided - SIGUSR2 won't be sent.\n");
    }

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
        if (searchRecord(currRecord, pattern)) printRecord(currRecord);
    }
    fclose(datafp);

    bool isOutRedirected = !(bool) isatty(STDOUT_FILENO);
    if (isOutRedirected) printf("%s", termSequence);

    Statistics stats;
    stats.cpuTime = ((double) (clock() - start_t)) / CLOCKS_PER_SEC;
    stats.recordsNum = rangeEnd - rangeStart + 1;
    if (isOutRedirected) {
        // then write stats in binary, as they will be read by a splitter-merger
        fwrite(&stats, sizeof(Statistics), 1, stdout);
        printf("%s", termSequence);
    } else {
        printf("Statistics:\n");
        printf("Reocrds Searched: %d\n", stats.recordsNum);
        printf("CPU Time: %f sec\n", stats.cpuTime);
    }

    if (argc == 6) kill(rootPid, SIGUSR2);
    return EC_OK;
}
