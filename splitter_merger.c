#define _GNU_SOURCE
#include <poll.h>
#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>

#include "headers/record.h"
#include "headers/statistics.h"
#include "headers/util.h"

/* Expected argv arguments, in that order:
 * datafile, rangeStart, rangeEnd, searchPattern, height, skew, rootPid */
int main(int argc, char *argv[]) {
    long long startTime = getCurrentTime();
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
    bool skew = (bool) strtol(argv[6], &strtolEndptr, 10);
    if (*strtolEndptr != '\0') {
        fprintf(stderr, "[Splitter-Merger] Invalid arguments: Skew can only be 0 or 1.\n");
        return EC_ARG;
    }
    int rootPid = (int) strtol(argv[7], &strtolEndptr, 10);
    if (*strtolEndptr != '\0' || rootPid < 2) {
        fprintf(stderr, "[Splitter-Merger] Invalid arguments: invalid Root pid.\n");
        return EC_ARG;
    }

    int fd1[2];
    pipe(fd1);
    int rangeSplit = (rangeStart + rangeEnd) / 2;         // TODO: assuming skew == 0
    char rangeStartStr[MAX_NUM_STRING_SIZE];
    char rangeEndStr[MAX_NUM_STRING_SIZE];
    char rootPidStr[MAX_NUM_STRING_SIZE];
    sprintf(rootPidStr, "%d", rootPid);
    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("[Splitter-Merger] fork");
        return EC_FORK;
    }
    if (pid1 == 0) {         // first child
        dup2(fd1[WRITE_END], STDOUT_FILENO);
        close(fd1[READ_END]);
        close(fd1[WRITE_END]);
        sprintf(rangeStartStr, "%d", rangeStart);
        sprintf(rangeEndStr, "%d", rangeSplit);
        if (height == 1) {
            execlp("./searcher", "searcher", datafile, rangeStartStr,
                   rangeEndStr, pattern, rootPidStr, (char *) NULL);
        } else {
            char heightStr[2];
            sprintf(heightStr, "%d", height - 1);
            char skewStr[2];
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
    if (pid2 < 0) {
        perror("[Splitter-Merger] fork");
        return EC_FORK;
    }
    if (pid2 == 0) {         // second child
        close(fd1[READ_END]);
        dup2(fd2[WRITE_END], STDOUT_FILENO);
        close(fd2[READ_END]);
        close(fd2[WRITE_END]);
        sprintf(rangeStartStr, "%d", rangeSplit + 1);
        sprintf(rangeEndStr, "%d", rangeEnd);
        if (height == 1) {
            execl("./searcher", "searcher", datafile, rangeStartStr,
                   rangeEndStr, pattern, rootPidStr, (char *) NULL);
        } else {
            char heightStr[2];
            sprintf(heightStr, "%d", height - 1);
            char skewStr[2];
            sprintf(skewStr, "%d", skew);
            execl("./splitter_merger", "splitter_merger", datafile, rangeStartStr,
                  rangeEndStr, pattern, heightStr, skewStr, rootPidStr, (char *) NULL);
        }
        perror("[Splitter-Merger] execl");
        return EC_EXEC;
    }
    close(fd2[WRITE_END]);

    struct pollfd pollfd[2];
    pollfd[0].fd = fd1[READ_END];
    pollfd[1].fd = fd2[READ_END];
    pollfd[0].events = pollfd[1].events = POLLIN;
    pollfd[0].revents = pollfd[1].revents = 0;

    bool child_completed[2] = {false, false};
    int nextStructIndicator;
    Record currRecord;
    SearcherStats searcherStats[2];
    SMStats smStats[2];
    while (!child_completed[0] || !child_completed[1]) {
        if (poll(pollfd, (nfds_t) 2, -1) < 0) {
            perror("[Splitter-Merger] poll");
            return EC_PIPE;
        }
        for (int i = 0; i < 2; i++) {       // poll both children
            if (pollfd[i].revents & POLLIN && !child_completed[i]) {       // we can read from child i
                if (read(pollfd[i].fd, &nextStructIndicator, sizeof(int)) < 0) {
                    perror("[Splitter-Merger] Error reading from pipe");
                    return EC_PIPE;
                }
                if (nextStructIndicator == 0) {     // Next struct is a Record
                    if (read(pollfd[i].fd, &currRecord, sizeof(Record)) < 0) {
                        perror("[Splitter-Merger] Error reading from pipe");
                        return EC_PIPE;
                    }
                    fwrite(&nextStructIndicator, sizeof(int), 1, stdout);
                    fwrite(&currRecord, sizeof(Record), 1, stdout);
                    fflush(stdout);
                } else if (nextStructIndicator == 1) {        // Next struct is SearcherStats
                    if (read(pollfd[i].fd, &searcherStats[i], sizeof(SearcherStats)) < 0) {
                        perror("[Splitter-Merger] Error reading from pipe");
                        return EC_PIPE;
                    }
                    child_completed[i] = true;
                } else if (nextStructIndicator == 2) {        // Next struct is SMStats
                    if (read(pollfd[i].fd, &smStats[i], sizeof(SMStats)) < 0) {
                        perror("[Splitter-Merger] Error reading from pipe");
                        return EC_PIPE;
                    }
                    child_completed[i] = true;
                } else {        // should never get here
                    fprintf(stderr, "[Splitter-Merger] Received invalid data from pipe.\n");
                    return EC_INVALID;
                }
            }
        }
    }

    double selfTime = (getCurrentTime() - startTime) / 1000.0;       // in seconds
    SMStats *currSMStats;
    if (height == 1) {
        currSMStats = combineSearcherStats(searcherStats[0], searcherStats[1], selfTime);
    } else {
        currSMStats = combineSMStats(smStats[0], smStats[1], selfTime);
    }
    if (currSMStats == NULL) {
        perror("[Splitter-Merger] malloc");
        return EC_MEM;
    }
    nextStructIndicator = 2;
    fwrite(&nextStructIndicator, sizeof(int), 1, stdout);
    fwrite(currSMStats, sizeof(SMStats), 1, stdout);
    free(currSMStats);
    fflush(stdout);

    close(fd1[READ_END]);
    close(fd2[READ_END]);
    // wait for both children:
    wait(NULL);
    wait(NULL);
    return EC_OK;
}
