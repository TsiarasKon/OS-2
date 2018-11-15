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
        perror("[Root] stat");
        return EC_FILE;
    }
    if (st.st_size == 0) {
        fprintf(stderr, "[Root] Warning: Datafile is empty.\n");
    }
    int recordsNum = (int) (st.st_size / sizeof(Record));
    // Check if not whole division - recordsNum wouldn't have been an integer:
    if (recordsNum != (st.st_size / (double) sizeof(Record))) {
        fprintf(stderr, "[Root] Invalid Datafile format: Datafile's size is not a multiple of Record's size.");
        return EC_INVALID;
    }

    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = sigusr2Handler;
    act.sa_flags = SA_RESTART;      // so that read()s won't be interrupted
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
        char rangeStartStr[MAX_NUM_STRING_SIZE];
        sprintf(rangeStartStr, "%d", 0);
        char rangeEndStr[MAX_NUM_STRING_SIZE];
        sprintf(rangeEndStr, "%d", recordsNum - 1);
        char rootPidStr[MAX_NUM_STRING_SIZE];
        sprintf(rootPidStr, "%d", getppid());
        char heightStr[2];
        sprintf(heightStr, "%d", height);
        char skewStr[2];
        sprintf(skewStr, "%d", skew);
        execl("./splitter_merger", "splitter_merger", datafile, rangeStartStr,
              rangeEndStr, pattern, heightStr, skewStr, rootPidStr, (char *) NULL);
        // this code will run only if exec fails:
        perror("[Root] execl");
        return EC_EXEC;
    }
    close(smfd[WRITE_END]);

    printf("Searching Records from '%s' containing \"%s\" in any field ...\n",
            datafile, pattern);

    RecordList *recordList = createRecordList();
    if (recordList == NULL) {
        perror("[Root] malloc");
        return EC_MEM;
    }
    int nextStructIndicator = 0;
    Record currRecord;
    SMStats completeSMStats;
    bool smDone = false;
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
    wait(NULL);      // wait for "root" Spiltter-Merger to complete

    if (recordList->first == NULL) {
        printf("No records matched!\n");
    } else if (recordList->first == recordList->last) {     // only one result - no reason to sort
        printf("Matching Records:\n");
        printRecordList(stdout, recordList);
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
            execlp("/usr/bin/sort", "sort", "-n", (char *) NULL);
            perror("[Root] execlp");
            deleteRecordList(&recordList);
            return EC_EXEC;
        }
        FILE* sorterfp = fdopen(sorterfd[WRITE_END], "w");
        printRecordList(sorterfp, recordList);
        fclose(sorterfp);
        wait(NULL);
    }
    deleteRecordList(&recordList);

    printf("\nStats:\n");
    printSMStats(completeSMStats);
    printf("SIGUSR2 received: %d\n", sigusr2Received);

    return EC_OK;
}
