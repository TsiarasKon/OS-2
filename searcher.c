#define _GNU_SOURCE
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "headers/record.h"
#include "headers/statistics.h"
#include "headers/util.h"

/* Expected argv arguments, in that order:
 * datafile, rangeStart, rangeEnd, searchPattern, rootPid */
int main(int argc, char *argv[]) {
    long long startTime = getCurrentTime();
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
            fflush(stdout);
            stats.recordsMatched++;
        }
    }
    fclose(datafp);

    // Write stats to stdout:
    stats.searcherTime = (getCurrentTime() - startTime) / 1000.0;       // in seconds
    nextStructIndicator = 1;
    fwrite(&nextStructIndicator, sizeof(int), 1, stdout);
    fwrite(&stats, sizeof(SearcherStats), 1, stdout);

    kill(rootPid, SIGUSR2);
    return EC_OK;
}
