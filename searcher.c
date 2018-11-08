#define _POSIX_SOURCE
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include "headers/util.h"
#include "headers/statistics.h"
#include "headers/record.h"

/* Expected argv arguments, in that order:
 * datafile, rangeStart, rangeEnd, searchPattern, rootPid */
int main(int argc, char *argv[]) {
    clock_t start_t = clock();
    if (argc != 6) {
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
    int rootPid = (int) strtol(argv[5], &strtolEndptr, 10);
    if (*strtolEndptr != '\0' || rootPid < 2) {
        fprintf(stderr, "[Searcher] Invalid arguments: invalid Root pid.\n");
        return EC_ARG;
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
    if ( st.st_size < (unsigned int) ((rangeEnd + 1) * sizeof(Record)) ) {
        fprintf(stderr, "[Searcher] rangeEnd surpasses file end.\n");
        return EC_INVALID;
    }
    fseek(datafp, rangeStart * sizeof(Record), SEEK_SET);       // move file position to rangeStart Record

    SearcherStats stats;
    stats.recordsMatched = 0;
    int nextStructIndicator = 0;        // 0 for Record, 1 for Statistics
    Record currRecord;
    for (int i = rangeStart; i <= rangeEnd; i++) {
        fread(&currRecord, sizeof(Record), 1, datafp);
        if (searchRecord(currRecord, pattern)) {
            // if Record matched Pattern, write it to stdout
            fwrite(&nextStructIndicator, sizeof(int), 1, stdout);
            fwrite(&currRecord, sizeof(Record), 1, stdout);
            stats.recordsMatched++;
        }
    }
    fclose(datafp);

    // write stats to stdout
    stats.cpuTime = ((double) (clock() - start_t)) / CLOCKS_PER_SEC;
    nextStructIndicator = 1;
    fwrite(&nextStructIndicator, sizeof(int), 1, stdout);
    fwrite(&stats, sizeof(SearcherStats), 1, stdout);

//    /// DEBUG:
//    fprintf(stderr, "Statistics:\n");
//    fprintf(stderr, "Records Matched: %d\n", stats.recordsMatched);
//    fprintf(stderr, "CPU Time: %f sec\n", stats.cpuTime);

//    kill(rootPid, SIGUSR2);
    return EC_OK;
}
