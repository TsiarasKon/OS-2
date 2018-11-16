#ifndef OS_2_RECORD_H
#define OS_2_RECORD_H

#include <stdbool.h>
#include <stdio.h>

typedef struct {
    long am;
    char fisrtName[20];
    char lastName[20];
    char street[20];
    int streetNum;
    char city[20];
    char zipCode[6];
    float salary;
} Record;

bool searchRecord(Record r, char *pattern);
void printRecord(FILE *fp, Record r);

/* Record List definitions and functions declarations: */

typedef struct recordListNode RecordListNode;
struct recordListNode {
    Record *record;
    RecordListNode *next;
};

typedef struct {
    RecordListNode *first;
    RecordListNode *last;
} RecordList;

RecordList *createRecordList(void);
void deleteRecordList(RecordList **rList);
bool addRecordToList(RecordList *rList, Record r);
void printRecordList(FILE *fp, RecordList *rList);
void prettyPrintResultSepLine(void);
void prettyPrintResultHeader(void);

#endif
