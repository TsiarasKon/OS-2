#ifndef OS_2_STATISTICS_H
#define OS_2_STATISTICS_H

typedef struct {
    double searcherTime;
    long recordsSearched;
    long recordsMatched;
} SearcherStats;

typedef struct {
    int totalSearchersNum;
    int totalSMNum;
    long totalRecordsSearched;
    long totalRecordsMatched;
    double minSearcherTime;
    double maxSearcherTime;
    double avgSearcherTime;
    long minSearcherRecordsSearched;
    long maxSearcherRecordsSearched;
    double avgSearcherRecordsSearched;
    long minSearcherRecordsMatched;
    long maxSearcherRecordsMatched;
    double avgSearcherRecordsMatched;
    double minSMTime;
    double maxSMTime;
    double avgSMTime;
} SMStats;

SMStats *combineSearcherStats(SearcherStats st1, SearcherStats st2, double selfTime);
SMStats *combineSMStats(SMStats st1, SMStats st2, double selfTime);
void prettyPrintRootStats(SMStats smStats, int sigusr2Received, double sorterTime, double turnaroundTime);

#endif
