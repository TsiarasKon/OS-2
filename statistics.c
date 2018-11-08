#include <stdio.h>
#include <stdlib.h>
#include "headers/statistics.h"

SMStats *combineSearcherStats(SearcherStats st1, SearcherStats st2, double selfCpuTime) {
    SMStats *newSMStats = malloc(sizeof(newSMStats));
    if (st1.cpuTime < st2.cpuTime) {
        newSMStats->minSearcherCpuTime = st1.cpuTime;
        newSMStats->maxSearcherCpuTime = st2.cpuTime;
    } else {
        newSMStats->minSearcherCpuTime = st2.cpuTime;
        newSMStats->maxSearcherCpuTime = st1.cpuTime;
    }
    newSMStats->avgSearcherCpuTime = (st1.cpuTime + st2.cpuTime) / 2;
    if (st1.recordsMatched < st2.recordsMatched) {
        newSMStats->minSearcherRecordsMatched = st1.recordsMatched;
        newSMStats->maxSearcherRecordsMatched = st2.recordsMatched;
    } else {
        newSMStats->minSearcherRecordsMatched = st2.recordsMatched;
        newSMStats->maxSearcherRecordsMatched = st1.recordsMatched;
    }
    newSMStats->avgSearcherRecordsMatched = (st1.recordsMatched + st2.recordsMatched) / 2;
    newSMStats->minSMCpuTime = newSMStats->maxSMCpuTime = newSMStats->avgSMCpuTime = selfCpuTime;
    return newSMStats;
}

void printSMStats(SMStats st) {
    printf("Splitter-Merger Stats:\n");
//    printf(" Min Searcher CPU Time: %f\n", st.minSearcherCpuTime);
//    printf(" Max Searcher CPU Time: %f\n", st.maxSearcherCpuTime);
//    printf(" Average Searcher CPU Time: %f\n", st.avgSearcherCpuTime);
//    printf(" Min Searcher Records matched: %d\n", st.minSearcherRecordsMatched);
//    printf(" Max Searcher Records matched: %d\n", st.maxSearcherRecordsMatched);
//    printf(" Average Searcher Records matched: %d\n", st.avgSearcherRecordsMatched);
//    printf(" Min Splitter-Merger CPU Time: %f\n", st.minSMCpuTime);
//    printf(" Max Splitter-Merger CPU Time: %f\n", st.maxSMCpuTime);
//    printf(" Average Splitter-Merger CPU Time: %f\n", st.avgSMCpuTime);
    printf("                          |    Min   |    Max   |    Avg   |\n");
    printf("      Searcher Times      | %8f | %8f | %8f |\n",
            st.minSearcherCpuTime, st.maxSearcherCpuTime, st.avgSearcherCpuTime);
    printf(" Searcher Records Matched | %8d | %8d | %8d |\n",
            st.minSearcherRecordsMatched, st.maxSearcherRecordsMatched, st.avgSearcherRecordsMatched);
    printf("   Splitter-Merger Times  | %8f | %8f | %8f |\n",
            st.minSMCpuTime, st.maxSMCpuTime, st.avgSMCpuTime);
}
