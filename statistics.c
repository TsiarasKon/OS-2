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

void printSMStats(SMStats st) {
    printf("Splitter-Merger Stats:\n");
//    printf(" Min Searcher Time: %f\n", st.minSearcherTime);
//    printf(" Max Searcher Time: %f\n", st.maxSearcherTime);
//    printf(" Average Searcher Time: %f\n", st.avgSearcherTime);
//    printf(" Min Searcher Records matched: %d\n", st.minSearcherRecordsMatched);
//    printf(" Max Searcher Records matched: %d\n", st.maxSearcherRecordsMatched);
//    printf(" Average Searcher Records matched: %d\n", st.avgSearcherRecordsMatched);
//    printf(" Min Splitter-Merger Time: %f\n", st.minSMTime);
//    printf(" Max Splitter-Merger Time: %f\n", st.maxSMTime);
//    printf(" Average Splitter-Merger Time: %f\n", st.avgSMTime);
    printf("                          |    Min   |    Max   |    Avg   |\n");
    printf("      Searcher Times      | %8f | %8f | %8f |\n",
            st.minSearcherTime, st.maxSearcherTime, st.avgSearcherTime);
    printf(" Searcher Records Matched | %8d | %8d | %8.3f |\n",
            st.minSearcherRecordsMatched, st.maxSearcherRecordsMatched, st.avgSearcherRecordsMatched);
    printf("   Splitter-Merger Times  | %8f | %8f | %8f |\n",
            st.minSMTime, st.maxSMTime, st.avgSMTime);
    printf("      Searchers Number    | %30d |\n", st.totalSearchersNum);
    printf("   Total Records Matched  | %30d |\n", st.totalRecordsMatched);
}
