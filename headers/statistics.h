#ifndef OS_2_STATISTICS_H
#define OS_2_STATISTICS_H

typedef struct {
    double searcherTime;
    int recordsMatched;
} SearcherStats;

typedef struct {
    int totalSearchersNum;
    int totalSMNum;
    int totalRecordsMatched;
    double minSearcherTime;
    double maxSearcherTime;
    double avgSearcherTime;
    int minSearcherRecordsMatched;
    int maxSearcherRecordsMatched;
    double avgSearcherRecordsMatched;
    double minSMTime;
    double maxSMTime;
    double avgSMTime;
} SMStats;

SMStats *combineSearcherStats(SearcherStats st1, SearcherStats st2, double selfTime);
SMStats *combineSMStats(SMStats st1, SMStats st2, double selfTime);
void printSMStats(SMStats st);

#endif
