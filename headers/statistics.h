#ifndef OS_2_STATISTICS_H
#define OS_2_STATISTICS_H

typedef struct {
    double searcherTime;
    long recordsMatched;
} SearcherStats;

typedef struct {
    int totalSearchersNum;
    int totalSMNum;
    long totalRecordsMatched;
    double minSearcherTime;
    double maxSearcherTime;
    double avgSearcherTime;
    long minSearcherRecordsMatched;
    long maxSearcherRecordsMatched;
    double avgSearcherRecordsMatched;
    double minSMTime;
    double maxSMTime;
    double avgSMTime;
} SMStats;

SMStats *combineSearcherStats(SearcherStats st1, SearcherStats st2, double selfTime);
SMStats *combineSMStats(SMStats st1, SMStats st2, double selfTime);
void printSMStats(SMStats smStats);
void printRootStats(SMStats smStats, long totalRecordsNum, int sigusr2Received, double turnaroundTime);

#endif
