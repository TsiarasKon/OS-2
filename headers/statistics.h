#ifndef OS_2_STATISTICS_H
#define OS_2_STATISTICS_H

typedef struct {
    double cpuTime;
    int recordsMatched;
} SearcherStats;

typedef struct {
    double minSearcherCpuTime;
    double maxSearcherCpuTime;
    double avgSearcherCpuTime;
    int minSearcherRecordsMatched;
    int maxSearcherRecordsMatched;
    int avgSearcherRecordsMatched;
    double minSMCpuTime;
    double maxSMCpuTime;
    double avgSMCpuTime;
} SMStats;

SMStats *combineSearcherStats(SearcherStats st1, SearcherStats st2, double selfCpuTime);
SMStats *combineSMStats(SMStats st1, SMStats st2, double selfCpuTime);
void printSMStats(SMStats st);

#endif
