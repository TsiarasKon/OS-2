#include "headers/record.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "headers/util.h"

bool searchRecord(Record r, char *pattern) {
    if ( strstr(r.fisrtName, pattern) || strstr(r.lastName, pattern) ||
            strstr(r.street, pattern) || strstr(r.city, pattern) ||
            strstr(r.zipCode, pattern) )
        return true;
    // Checking numeric fields as well:
    char numStr[MAX_NUM_STRING_SIZE];
    sprintf(numStr, "%ld", r.am);
    if ( strstr(numStr, pattern) ) return true;
    sprintf(numStr, "%d", r.streetNum);
    if ( strstr(numStr, pattern) ) return true;
    sprintf(numStr, "%f", r.salary);
    if ( strstr(numStr, pattern) ) return true;
    return false;
}

bool readRecord(int fd, Record *record) {       // TODO: improve and use
    if (read(fd, record, sizeof(Record)) < 0) {
        return false;
    }
    return true;
}

void printRecord(FILE *fp, Record r) {
    if (fp == NULL) fp = stdout;
    fprintf(fp, "%ld | %s %s | %s %d %s %s | %f\n", r.am, r.fisrtName, r.lastName,
           r.street, r.streetNum, r.city, r.zipCode, r.salary);
}

/* Record List functions: */

RecordList *createRecordList() {
    RecordList *rList = malloc(sizeof(RecordList));
    if (rList == NULL) return NULL;
    rList->first = rList->last = NULL;
    return rList;
}

void deleteRecordList(RecordList **rList) {
    if (*rList == NULL) {
        fprintf(stderr, "Attempted to delete NULL RecordList.\n");
        return;
    }
    RecordListNode *currentR = (*rList)->first;
    RecordListNode *nextR;
    while (currentR != NULL) {
        nextR = currentR->next;
        free(currentR->record);
        free(currentR);
        currentR = nextR;
    }
    free(*rList);
    *rList = NULL;
}

bool addRecordToList(RecordList *rList, Record r) {
    if (rList == NULL) return false;
    RecordListNode *newR = malloc(sizeof(RecordListNode));
    if (newR == NULL) return false;
    newR->record = malloc(sizeof(Record));
    if (newR->record == NULL) return false;
    memcpy(newR->record, &r, sizeof(Record));
    if (rList->first == NULL) {
        rList->first = rList->last = newR;
    } else {
        rList->last->next = newR;
        rList->last = rList->last->next;
    }
    return true;
}

void printRecordList(FILE *fp, RecordList *rList) {
    if (fp == NULL) fp = stdout;
    RecordListNode *currentR = rList->first;
    while (currentR != NULL) {
        printRecord(fp, *(currentR->record));
        currentR = currentR->next;
    }
}
