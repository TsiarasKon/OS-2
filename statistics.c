#include <stdio.h>
#include <stdlib.h>
#include "headers/statistics.h"

SMStats *combineSearcherStats(SearcherStats st1, SearcherStats st2, double selfCpuTime) {
    SMStats *newSMStats = malloc(sizeof(SMStats));
    if (newSMStats == NULL) return NULL;
    newSMStats->totalSearchersNum = 2;
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
    newSMStats->totalRecordsMatched = st1.recordsMatched + st2.recordsMatched;
    newSMStats->minSMCpuTime = newSMStats->maxSMCpuTime = newSMStats->avgSMCpuTime = selfCpuTime;
    return newSMStats;
}

SMStats *combineSMStats(SMStats st1, SMStats st2, double selfCpuTime) {
    SMStats *newSMStats = malloc(sizeof(SMStats));
    if (newSMStats == NULL) return NULL;
    newSMStats->totalSearchersNum = st1.totalSearchersNum + st2.totalSearchersNum;
    newSMStats->minSearcherCpuTime = (st1.minSearcherCpuTime < st2.minSearcherCpuTime) ?
            st1.minSearcherCpuTime : st2.minSearcherCpuTime;
    newSMStats->maxSearcherCpuTime = (st1.maxSearcherCpuTime > st2.minSearcherCpuTime) ?
            st1.maxSearcherCpuTime : st2.maxSearcherCpuTime;
    newSMStats->avgSearcherCpuTime = (st1.avgSearcherCpuTime + st2.avgSearcherCpuTime) / 2;
    newSMStats->minSearcherRecordsMatched = (st1.minSearcherRecordsMatched < st2.minSearcherRecordsMatched) ?
            st1.minSearcherRecordsMatched : st2.minSearcherRecordsMatched;
    newSMStats->maxSearcherRecordsMatched = (st1.maxSearcherRecordsMatched > st2.maxSearcherRecordsMatched) ?
            st1.maxSearcherRecordsMatched : st2.maxSearcherRecordsMatched;
    newSMStats->avgSearcherRecordsMatched = (st1.avgSearcherRecordsMatched + st2.avgSearcherRecordsMatched) / 2;
    newSMStats->totalRecordsMatched = st1.totalRecordsMatched + st2.totalRecordsMatched;
    newSMStats->minSMCpuTime = (st1.minSMCpuTime < st2.minSMCpuTime) ?
            st1.minSMCpuTime : st2.minSMCpuTime;
    newSMStats->maxSMCpuTime = selfCpuTime;
    newSMStats->avgSMCpuTime = (newSMStats->avgSMCpuTime + selfCpuTime) / 2;         // TODO: maybe not this
    return newSMStats;
}

void printSMStats(SMStats st) {
    fprintf(stderr, "Splitter-Merger Stats:\n");
//    printf(" Min Searcher CPU Time: %f\n", st.minSearcherCpuTime);
//    printf(" Max Searcher CPU Time: %f\n", st.maxSearcherCpuTime);
//    printf(" Average Searcher CPU Time: %f\n", st.avgSearcherCpuTime);
//    printf(" Min Searcher Records matched: %d\n", st.minSearcherRecordsMatched);
//    printf(" Max Searcher Records matched: %d\n", st.maxSearcherRecordsMatched);
//    printf(" Average Searcher Records matched: %d\n", st.avgSearcherRecordsMatched);
//    printf(" Min Splitter-Merger CPU Time: %f\n", st.minSMCpuTime);
//    printf(" Max Splitter-Merger CPU Time: %f\n", st.maxSMCpuTime);
//    printf(" Average Splitter-Merger CPU Time: %f\n", st.avgSMCpuTime);
    fprintf(stderr, "                          |    Min   |    Max   |    Avg   |\n");
    fprintf(stderr, "      Searcher Times      | %8f | %8f | %8f |\n",
            st.minSearcherCpuTime, st.maxSearcherCpuTime, st.avgSearcherCpuTime);
    fprintf(stderr, " Searcher Records Matched | %8d | %8d | %8d |\n",
            st.minSearcherRecordsMatched, st.maxSearcherRecordsMatched, st.avgSearcherRecordsMatched);
    fprintf(stderr, "   Splitter-Merger Times  | %8f | %8f | %8f |\n",
            st.minSMCpuTime, st.maxSMCpuTime, st.avgSMCpuTime);
    fprintf(stderr, "      Searchers Number    | %30d |\n", st.totalSearchersNum);
    fprintf(stderr, "   Total Records Matched  | %30d |\n", st.totalRecordsMatched);
}
