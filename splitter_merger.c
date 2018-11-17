#define _GNU_SOURCE
#include <poll.h>
#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "headers/record.h"
#include "headers/statistics.h"
#include "headers/util.h"

/* Expected argv arguments, in that order:
 * datafile, searchersNum, firstSearcher, lastSearcher, searchPattern, height, skew, rootPid */
int main(int argc, char *argv[]) {
    long long startTime = getCurrentTime();
    if (argc != 9) {
        fprintf(stderr, "[Splitter-Merger] Invalid number of arguments.\n");
        return EC_ARG;
    }
    char *datafile = argv[1];
    char *strtolEndptr = NULL;
    int searchersNum = (int) strtol(argv[2], &strtolEndptr, 10);
    if (*strtolEndptr != '\0' || searchersNum < 2) {
        fprintf(stderr, "[Splitter-Merger] Invalid arguments: searchersNum must be an integer greater than 1.\n");
        return EC_ARG;
    }
    int firstSearcher = (int) strtol(argv[3], &strtolEndptr, 10);
    if (*strtolEndptr != '\0' || firstSearcher < 0) {
        fprintf(stderr, "[Splitter-Merger] Invalid arguments: firstSearcher must be a non-negative integer.\n");
        return EC_ARG;
    }
    int lastSearcher = (int) strtol(argv[4], &strtolEndptr, 10);
    if (*strtolEndptr != '\0' || lastSearcher <= firstSearcher) {
        fprintf(stderr, "[Splitter-Merger] Invalid arguments: lastSearcher must be an integer greater than firstSearcher.\n");
        return EC_ARG;
    }
    char *pattern = argv[5];
    int height = (int) strtol(argv[6], &strtolEndptr, 10);
    if (*strtolEndptr != '\0' || height < 1 || height > 5) {
        fprintf(stderr, "[Splitter-Merger] Invalid arguments: Height must be an integer between 1 and 5 (inclusive).\n");
        return EC_ARG;
    }
    bool skew = (bool) strtol(argv[7], &strtolEndptr, 10);
    if (*strtolEndptr != '\0') {
        fprintf(stderr, "[Splitter-Merger] Invalid arguments: Skew can only be 0 or 1.\n");
        return EC_ARG;
    }
    pid_t rootPid = (pid_t) strtol(argv[8], &strtolEndptr, 10);
    if (*strtolEndptr != '\0' || rootPid < 2) {
        fprintf(stderr, "[Splitter-Merger] Invalid arguments: invalid Root pid.\n");
        return EC_ARG;
    }

    int fd1[2];
    pipe(fd1);
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
        if (height == 1) {      // child will be a Searcher
            struct stat st;
            if (stat(datafile, &st) != 0) {
                perror("[Splitter-Merger] stat");
                return EC_FILE;
            }
            long recordsNum = st.st_size / sizeof(Record);
            long rangeStart, rangeEnd;
            if (! skew) {       // as per exercise description, each worker gets k/2^h
                rangeStart = (long) (firstSearcher * (recordsNum / (double) searchersNum));
                rangeEnd = (long) ((firstSearcher + 1) * (recordsNum / (double) searchersNum) - 1);
            } else {            // if skew was specified, each worker gets k*i/sumToN(2^h)
                rangeStart = (long) (recordsNum * (sumToN(firstSearcher) / (double) sumToN(searchersNum)));
                rangeEnd = (long) (recordsNum * (sumToN((firstSearcher + 1)) / (double) sumToN(searchersNum)) - 1);
            }
            char rangeStartStr[MAX_NUM_STRING_SIZE];
            sprintf(rangeStartStr, "%ld", rangeStart);
            char rangeEndStr[MAX_NUM_STRING_SIZE];
            sprintf(rangeEndStr, "%ld", rangeEnd);
            execl("./searcher", "searcher", datafile, rangeStartStr,
                   rangeEndStr, pattern, rootPidStr, (char *) NULL);
        } else {        // child will be another Splitter-Merger
            char lastSearcherStr[MAX_NUM_STRING_SIZE];
            sprintf(lastSearcherStr, "%d", (firstSearcher + lastSearcher) / 2);
            char heightStr[MAX_NUM_STRING_SIZE];
            sprintf(heightStr, "%d", height - 1);
            char skewStr[2];
            sprintf(skewStr, "%d", skew);
            execl("./splitter_merger", "splitter_merger", datafile, argv[2], argv[3],
                lastSearcherStr, pattern, heightStr, skewStr, rootPidStr, (char *) NULL);
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
        if (height == 1) {      // child will be a Searcher
            struct stat st;
            if (stat(datafile, &st) != 0) {
                perror("[Splitter-Merger] stat");
                return EC_FILE;
            }
            long recordsNum = st.st_size / sizeof(Record);
            long rangeStart, rangeEnd;
            if (! skew) {
                rangeStart = (long) (lastSearcher * (recordsNum / (double) searchersNum));
                rangeEnd = (long) ((lastSearcher + 1) * (recordsNum / (double) searchersNum) - 1);
            } else {
                rangeStart = (long) (recordsNum * (sumToN(lastSearcher) / (double) sumToN(searchersNum)));
                rangeEnd = min( (long) (recordsNum * (sumToN(lastSearcher + 1) / (double) sumToN(searchersNum)) - 1),
                       recordsNum - 1);     // last Searcher should receive remaining Records until the end
            }
            char rangeStartStr[MAX_NUM_STRING_SIZE];
            sprintf(rangeStartStr, "%ld", rangeStart);
            char rangeEndStr[MAX_NUM_STRING_SIZE];
            sprintf(rangeEndStr, "%ld", rangeEnd);
            execl("./searcher", "searcher", datafile, rangeStartStr,
                   rangeEndStr, pattern, rootPidStr, (char *) NULL);
        } else {        // child will be another Splitter-Merger
            char firstSearcherStr[MAX_NUM_STRING_SIZE];
            sprintf(firstSearcherStr, "%d", (firstSearcher + lastSearcher) / 2 + 1);
            char heightStr[MAX_NUM_STRING_SIZE];
            sprintf(heightStr, "%d", height - 1);
            char skewStr[2];
            sprintf(skewStr, "%d", skew);
            execl("./splitter_merger", "splitter_merger", datafile, argv[2], firstSearcherStr,
                  argv[4], pattern, heightStr, skewStr, rootPidStr, (char *) NULL);
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
    /* poll() both children's pipes while they are still sending Records
     * If a pipe has data to read, read an int an then:
     * if 0 then also read a Record, write it as is to stdout and continue;
     * if 1 (can only be received if child is Searcher) then also read
     *      its Stats and mark it as completed;
     * if 2 (can only be received if children is Splitter-Merger) then also
     *      read its Stats and mark it as completed; */
    while (!child_completed[0] || !child_completed[1]) {
        if (poll(pollfd, (nfds_t) 2, -1) < 0) {
            perror("[Splitter-Merger] poll");
            return EC_PIPE;
        }
        for (int i = 0; i < 2; i++) {
            if (pollfd[i].revents & POLLIN && !child_completed[i]) {       // we can read from child i
                if (! readFromPipe(pollfd[i].fd, &nextStructIndicator, sizeof(int))) {
                    perror("[Splitter-Merger] Error reading from pipe");
                    return EC_PIPE;
                }
                if (nextStructIndicator == 0) {     // Next struct is a Record
                    if (! readFromPipe(pollfd[i].fd, &currRecord, sizeof(Record))) {
                        perror("[Splitter-Merger] Error reading from pipe");
                        return EC_PIPE;
                    }
                    fwrite(&nextStructIndicator, sizeof(int), 1, stdout);
                    fwrite(&currRecord, sizeof(Record), 1, stdout);
                    fflush(stdout);
                } else if (nextStructIndicator == 1) {        // Next struct is SearcherStats
                    if (! readFromPipe(pollfd[i].fd, &searcherStats[i], sizeof(SearcherStats))) {
                        perror("[Splitter-Merger] Error reading from pipe");
                        return EC_PIPE;
                    }
                    child_completed[i] = true;
                } else if (nextStructIndicator == 2) {        // Next struct is SMStats
                    if (! readFromPipe(pollfd[i].fd, &smStats[i], sizeof(SMStats))) {
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
    close(fd1[READ_END]);
    close(fd2[READ_END]);

    double selfTime = (getCurrentTime() - startTime) / 1000.0;       // in seconds
    /* Regardless of what its children were, combine their Stats
     * in a single new struct and write it to stdout */
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

    wait(NULL);
    wait(NULL);
    return EC_OK;
}
