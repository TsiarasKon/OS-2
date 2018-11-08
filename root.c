#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include "headers/util.h"
#include "headers/record.h"
#include "headers/statistics.h"

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

    // TODO: establish signal handler

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
        sprintf(rootPidStr, "%d", getpid());
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

    int nextStructIndicator = 0;
    Record currRecord;
    SMStats completeSMStats;
    bool smDone = false;
    while (!smDone) {
        if (read(smfd[READ_END], &nextStructIndicator, sizeof(int)) < 0) {
            perror("[Root] Error reading from pipe");
            return EC_PIPE;
        }
        if (nextStructIndicator == 0) {     // Next struct is a Record
            if (read(smfd[READ_END], &currRecord, sizeof(Record)) < 0) {
                perror("[Root] Error reading from pipe");
                return EC_PIPE;
            }
            printRecord(currRecord);
        } else if (nextStructIndicator == 2) {        // Next struct is SMStats
            if (read(smfd[READ_END], &completeSMStats, sizeof(SMStats)) < 0) {
                perror("[Root] Error reading from pipe");
                return EC_PIPE;
            }
            smDone = true;
        } else {        // should never get here
            printf("%d\n", nextStructIndicator);
            fprintf(stderr, "[Root] Received invalid data from pipe.\n");
            return EC_INVALID;
        }
    }
    printSMStats(completeSMStats);


    // TODO create sorter
//    int sorterfd[2];
//    pipe(sorterfd);
//    pid_t sorterPid = fork();
//    if (sorterPid < 0) {
//        perror("[Root] fork");
//        // TODO: frees?
//        return EC_FORK;
//    } else if (sorterPid == 0) {
//        execl("../sorter", "sorter", /* search results, */ (char *) NULL);
//        perror("[Root] execl");
//        return EC_EXEC;
//    }

    return EC_OK;
}