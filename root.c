#define _GNU_SOURCE
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <wait.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "headers/record.h"
#include "headers/statistics.h"
#include "headers/util.h"

volatile sig_atomic_t sigusr2Received = 0;

void sigusr2Handler() {
    sigusr2Received++;
}

int main(int argc, char *argv[]) {
    long long rootStartTime = getCurrentTime();
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
                    return EC_ARG;
                }
                break;
            case 'd':
                datafile = optarg;
                break;
            case 'p':
                pattern = optarg;
                break;
            case 's':
                skew = true;
                break;
            default:
                printf("%s", argErrorMsg);
                return EC_ARG;
        }
    }
    if (height == 0 || datafile == NULL || pattern == NULL) {
        printf("%s", argErrorMsg);
        return EC_ARG;
    }

    struct stat st;
    if (stat(datafile, &st) != 0) {     // file may not exist, be inaccessible, etc
        perror("Datafile is inaccessible.");
        return EC_FILE;
    }
    if (st.st_size == 0) {
        fprintf(stderr, "[Root] Warning: Datafile is empty.\n");
    }
    long recordsNum = st.st_size / sizeof(Record);
    // Check if not whole division - recordsNum wouldn't have been an integer:
    if (recordsNum != (st.st_size / (double) sizeof(Record))) {
        fprintf(stderr, "Invalid Datafile format: Datafile's size is not a multiple of Record's size.");
        return EC_INVALID;
    }
    int searchersNum = (1 << height);      // == 2^height

    // setup signal handler to catch Searchers' SIGUSR2 signals
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = sigusr2Handler;
    act.sa_flags = SA_RESTART;      // so that system calls won't be interrupted
    if ( sigaction(SIGUSR2, &act, NULL) < 0 ) {
        perror("[Root] sigaction");
        return EC_SIG;
    }

    int smfd[2];
    pipe(smfd);
    pid_t smPid = fork();
    if (smPid < 0) {
        perror("[Root] fork");
        return EC_FORK;
    } else if (smPid == 0) {
        dup2(smfd[WRITE_END], STDOUT_FILENO);
        close(smfd[READ_END]);
        close(smfd[WRITE_END]);
        char searchersNumStr[MAX_NUM_STRING_SIZE];
        sprintf(searchersNumStr, "%d", searchersNum);
        char firstSearcherStr[2];
        sprintf(firstSearcherStr, "%d", 0);
        char lastSearcherStr[MAX_NUM_STRING_SIZE];
        sprintf(lastSearcherStr, "%d", searchersNum - 1);
        char rootPidStr[MAX_NUM_STRING_SIZE];
        sprintf(rootPidStr, "%d", getppid());
        char heightStr[MAX_NUM_STRING_SIZE];
        sprintf(heightStr, "%d", height);
        char skewStr[2];
        sprintf(skewStr, "%d", skew);
        execl("./splitter_merger", "splitter_merger", datafile, searchersNumStr, firstSearcherStr,
              lastSearcherStr, pattern, heightStr, skewStr, rootPidStr, (char *) NULL);
        // this code will run only if exec fails:
        perror("[Root] execl");
        return EC_EXEC;
    }
    close(smfd[WRITE_END]);

    printf("Searching Records from '%s' containing \"%s\" in any field ...\n\n", datafile, pattern);

    RecordList *recordList = createRecordList();
    if (recordList == NULL) {
        perror("[Root] malloc");
        return EC_MEM;
    }
    int nextStructIndicator = 0;
    Record currRecord;
    SMStats completeSMStats;
    bool smDone = false;
    /* While root Splitter-Merger is still writing Records read an int:
     * if 0 then also read a Record, add it to recordList and continue looping;
     * if 2 then read Splitter-Merger's stats and break the loop; */
    while (!smDone) {
        if (! readFromPipe(smfd[READ_END], &nextStructIndicator, sizeof(int))) {
            perror("[Root] Error reading from pipe");
            deleteRecordList(&recordList);
            return EC_PIPE;
        }
        if (nextStructIndicator == 0) {     // Next struct is a Record
            if (! readFromPipe(smfd[READ_END], &currRecord, sizeof(Record))) {
                perror("[Root] Error reading from pipe");
                deleteRecordList(&recordList);
                return EC_PIPE;
            }
            if (! addRecordToList(recordList, currRecord)) {
                perror("[Root] malloc");
                deleteRecordList(&recordList);
                return EC_MEM;
            }
        } else if (nextStructIndicator == 2) {        // Next struct is SMStats
            if (! readFromPipe(smfd[READ_END], &completeSMStats, sizeof(SMStats))) {
                perror("[Root] Error reading from pipe");
                deleteRecordList(&recordList);
                return EC_PIPE;
            }
            smDone = true;
        } else {        // should never get here
            fprintf(stderr, "[Root] Received invalid data from pipe.\n");
            deleteRecordList(&recordList);
            return EC_INVALID;
        }
    }
    wait(NULL);      // wait for root Spiltter-Merger to complete

    long long sorterStartTime = getCurrentTime();
    double sorterTime = -1;
    if (recordList->first == NULL) {
        printf("No records matched!\n");
    } else if (recordList->first == recordList->last) {     // only one result - no reason to sort
        printf("Matching Records:\n");
        if (PRETTY_PRINT_RESULTS) {
            prettyPrintResultSepLine();
            prettyPrintResultHeader();
        }
        printRecordList(stdout, recordList);
        if (PRETTY_PRINT_RESULTS) prettyPrintResultSepLine();
    } else {
        int sorterfd[2];
        pipe(sorterfd);
        pid_t sorterPid = fork();
        if (sorterPid < 0) {
            perror("[Root] fork");
            deleteRecordList(&recordList);
            return EC_FORK;
        } else if (sorterPid == 0) {
            dup2(sorterfd[READ_END], STDIN_FILENO);
            close(sorterfd[READ_END]);
            close(sorterfd[WRITE_END]);
            printf("Matching Records:\n");
            if (PRETTY_PRINT_RESULTS) {
                prettyPrintResultSepLine();
                prettyPrintResultHeader();
            }
            execlp("/usr/bin/sort", "sort", "-n", (char *) NULL);
            perror("[Root] execlp");
            deleteRecordList(&recordList);
            return EC_EXEC;
        }
        // write all the Records in reocrdList as strings to the pipe
        FILE* sorterfp = fdopen(sorterfd[WRITE_END], "w");
        printRecordList(sorterfp, recordList);
        fclose(sorterfp);
        wait(NULL);
        if (PRETTY_PRINT_RESULTS) prettyPrintResultSepLine();
        sorterTime = (getCurrentTime() - sorterStartTime) / 1000.0;
    }
    deleteRecordList(&recordList);

    double turnaroundTime = (getCurrentTime() - rootStartTime) / 1000.0;
    prettyPrintRootStats(completeSMStats, sigusr2Received, sorterTime, turnaroundTime);

    return EC_OK;
}
