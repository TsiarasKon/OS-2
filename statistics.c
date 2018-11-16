#include "headers/statistics.h"

#include <stdio.h>
#include <stdlib.h>

SMStats *combineSearcherStats(SearcherStats st1, SearcherStats st2, double selfTime) {
    SMStats *newSMStats = malloc(sizeof(SMStats));
    if (newSMStats == NULL) return NULL;
    newSMStats->totalSearchersNum = 2;
    newSMStats->totalSMNum = 1;
    newSMStats->totalRecordsMatched = st1.recordsMatched + st2.recordsMatched;
    if (st1.searcherTime < st2.searcherTime) {
        newSMStats->minSearcherTime = st1.searcherTime;
        newSMStats->maxSearcherTime = st2.searcherTime;
    } else {
        newSMStats->minSearcherTime = st2.searcherTime;
        newSMStats->maxSearcherTime = st1.searcherTime;
    }
    newSMStats->avgSearcherTime = (st1.searcherTime + st2.searcherTime) / 2;
    if (st1.recordsMatched < st2.recordsMatched) {
        newSMStats->minSearcherRecordsMatched = st1.recordsMatched;
        newSMStats->maxSearcherRecordsMatched = st2.recordsMatched;
    } else {
        newSMStats->minSearcherRecordsMatched = st2.recordsMatched;
        newSMStats->maxSearcherRecordsMatched = st1.recordsMatched;
    }
    newSMStats->avgSearcherRecordsMatched = (st1.recordsMatched + st2.recordsMatched) / 2.0;
    newSMStats->minSMTime = newSMStats->maxSMTime = newSMStats->avgSMTime = selfTime;
    return newSMStats;
}

SMStats *combineSMStats(SMStats st1, SMStats st2, double selfTime) {
    SMStats *newSMStats = malloc(sizeof(SMStats));
    if (newSMStats == NULL) return NULL;
    newSMStats->totalSearchersNum = st1.totalSearchersNum + st2.totalSearchersNum;
    newSMStats->totalSMNum = st1.totalSMNum + st2.totalSMNum + 1;
    newSMStats->totalRecordsMatched = st1.totalRecordsMatched + st2.totalRecordsMatched;
    newSMStats->minSearcherTime = (st1.minSearcherTime < st2.minSearcherTime) ?
            st1.minSearcherTime : st2.minSearcherTime;
    newSMStats->maxSearcherTime = (st1.maxSearcherTime > st2.minSearcherTime) ?
            st1.maxSearcherTime : st2.maxSearcherTime;
    newSMStats->avgSearcherTime = (st1.avgSearcherTime + st2.avgSearcherTime) / 2;
    newSMStats->minSearcherRecordsMatched = (st1.minSearcherRecordsMatched < st2.minSearcherRecordsMatched) ?
            st1.minSearcherRecordsMatched : st2.minSearcherRecordsMatched;
    newSMStats->maxSearcherRecordsMatched = (st1.maxSearcherRecordsMatched > st2.maxSearcherRecordsMatched) ?
            st1.maxSearcherRecordsMatched : st2.maxSearcherRecordsMatched;
    newSMStats->avgSearcherRecordsMatched = (st1.avgSearcherRecordsMatched + st2.avgSearcherRecordsMatched) / 2;
    newSMStats->minSMTime = (st1.minSMTime < st2.minSMTime) ?
            st1.minSMTime : st2.minSMTime;
    newSMStats->maxSMTime = selfTime;
    newSMStats->avgSMTime = (selfTime / newSMStats->totalSMNum) +
            ((st1.avgSMTime) * (st1.totalSMNum / (double) newSMStats->totalSMNum)) +
            ((st2.avgSMTime) * (st2.totalSMNum / (double) newSMStats->totalSMNum));
    return newSMStats;
}

void prettyPrintRootStats(SMStats smStats, long totalRecordsNum,
        int sigusr2Received, double sorterTime, double turnaroundTime) {
    printf("\nStatistics:\n");
    printf("===================================================\n");
    printf("    Splitter-Mergers Number   |  %d \n", smStats.totalSMNum);
    printf("---------------------------------------------------\n");
    printf("        Searchers Number      |  %d \n", smStats.totalSearchersNum);
    printf("---------------------------------------------------\n");
    printf("   Total Records in Datafile  |  %ld \n", totalRecordsNum);
    printf("---------------------------------------------------\n");
    printf("     Total Records Matched    |  %ld \n", smStats.totalRecordsMatched);
    printf("---------------------------------------------------\n");
    printf("                              | Min: %ld\n", smStats.minSearcherRecordsMatched);
    printf(" Records Matched per Searcher | Max: %ld\n", smStats.maxSearcherRecordsMatched);
    printf("                              | Avg: %.3f\n", smStats.avgSearcherRecordsMatched);
    printf("---------------------------------------------------\n");
    printf("                              | Min: %.3f sec\n", smStats.minSearcherTime);
    printf("        Searcher Times        | Max: %.3f sec\n", smStats.maxSearcherTime);
    printf("                              | Avg: %.3f sec\n", smStats.avgSearcherTime);
    printf("---------------------------------------------------\n");
    printf("                              | Min: %.3f sec\n", smStats.minSMTime);
    printf("     Splitter-Merger Times    | Max: %.3f sec\n", smStats.maxSMTime);
    printf("                              | Avg: %.3f sec\n", smStats.avgSMTime);
    printf("---------------------------------------------------\n");
    if (sorterTime < 0) {
        printf("          Sorter Time         |  N/A \n");
    } else {
        printf("          Sorter Time         |  %.3f sec \n", sorterTime);
    }
    printf("---------------------------------------------------\n");
    printf("        Turnaround Time       |  %.3f sec \n", turnaroundTime);
    printf("---------------------------------------------------\n");
    printf("   SIGUSR2 Signals Received   |  %d \n", sigusr2Received);
    printf("===================================================\n");
}
