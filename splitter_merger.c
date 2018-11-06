#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "common/util.h"
#include "record.h"

/* Expected argv arguments, in that order:
 * datafile, rangeStart, rangeEnd, searchPattern, height, skew, rootPid */
int main(int argc, char *argv[]) {
    if (argc != 8) {
        fprintf(stderr, "[Splitter-Merger] Invalid number of arguments.\n");
        return EC_ARG;
    }
    char *datafile = argv[1];
    char *strtolEndptr = NULL;
    int rangeStart = (int) strtol(argv[2], &strtolEndptr, 10);
    if (*strtolEndptr != '\0' || rangeStart < 0) {
        fprintf(stderr, "[Splitter-Merger] Invalid arguments: rangeStart must be a non-negative integer.\n");
        return EC_ARG;
    }
    int rangeEnd = (int) strtol(argv[3], &strtolEndptr, 10);
    if (*strtolEndptr != '\0' || rangeEnd < rangeStart) {
        fprintf(stderr, "[Splitter-Merger] Invalid arguments: rangeEnd must be an integer greater than or equal to rangeStart.\n");
        return EC_ARG;
    }
    char *pattern = argv[4];
    int height = (int) strtol(argv[5], &strtolEndptr, 10);
    if (*strtolEndptr != '\0' || height < 1 || height > 5) {
        fprintf(stderr, "[Splitter-Merger] Invalid arguments: Height must be an integer between 1 and 5 (inclusive).\n");
        return EC_ARG;
    }
    bool skew = argv[6];
    int rootPid = (int) strtol(argv[7], &strtolEndptr, 10);
    if (*strtolEndptr != '\0' || rootPid < 2) {
        fprintf(stderr, "[Splitter-Merger] Invalid arguments: invalid Root pid.\n");
        return EC_ARG;
    }

    int fd1[2];
    pipe(fd1);
    pid_t pid1 = fork();
    int rangeSplit = (rangeStart + rangeEnd) / 2;         // TODO: assuming skew == 0
    char rangeStartStr[MAX_NUM_STRING_SIZE];
    char rangeEndStr[MAX_NUM_STRING_SIZE];
    char rootPidStr[MAX_NUM_STRING_SIZE];
    sprintf(rootPidStr, "%d", rootPid);
    if (pid1 == 0) {         // first child
        dup2(fd1[WRITE_END], STDOUT_FILENO);
        close(fd1[READ_END]);
        close(fd1[WRITE_END]);
        sprintf(rangeStartStr, "%d", rangeStart);
        sprintf(rangeEndStr, "%d", rangeStart + rangeSplit);
        if (height == 1) {
            execlp("./searcher", "searcher", datafile, rangeStartStr,
                   rangeEndStr, pattern, rootPidStr, (char *) NULL);
        } else {
            char heightStr[MAX_NUM_STRING_SIZE];
            sprintf(heightStr, "%d", height - 1);
            char skewStr[MAX_NUM_STRING_SIZE];
            sprintf(skewStr, "%d", skew);
            execl("./splitter_merger", "splitter_merger", datafile, rangeStartStr,
                  rangeEndStr, pattern, heightStr, skewStr, rootPidStr, (char *) NULL);
        }
        perror("[Splitter-Merger] execl");
        return EC_EXEC;
    }
    close(fd1[WRITE_END]);
    int fd2[2];
    pipe(fd2);
    pid_t pid2 = fork();
    if (pid2 == 0) {         // second child
        close(fd1[READ_END]);
        dup2(fd2[WRITE_END], STDOUT_FILENO);
        close(fd2[READ_END]);
        close(fd2[WRITE_END]);
        sprintf(rangeStartStr, "%d", rangeStart + rangeSplit + 1);
        sprintf(rangeEndStr, "%d", rangeEnd);
        if (height == 1) {
            execlp("./searcher", "searcher", datafile, rangeStartStr,
                   rangeEndStr, pattern, rootPidStr, (char *) NULL);
        } else {
            char heightStr[MAX_NUM_STRING_SIZE];
            sprintf(heightStr, "%d", height - 1);
            char skewStr[MAX_NUM_STRING_SIZE];
            sprintf(skewStr, "%d", skew);
            execl("./splitter_merger", "splitter_merger", datafile, rangeStartStr,
                  rangeEndStr, pattern, height - 1, skew, rootPidStr, (char *) NULL);
        }
        perror("[Splitter-Merger] execl");
        return EC_EXEC;
    }
    close(fd2[WRITE_END]);



}