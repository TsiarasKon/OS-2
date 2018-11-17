#define _GNU_SOURCE
#include "headers/record.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "headers/util.h"

bool searchRecord(Record r, char *pattern) {
    /* Will return true if pattern is a substring of any of the record's fields
     * using strstr() [or strcasestr() if IGNORE_CASE is defined as true] */
    char *(*substrFunc)(const char *, const char *);
    substrFunc = (IGNORE_CASE) ? strcasestr : strstr;
    if ( substrFunc(r.fisrtName, pattern) || substrFunc(r.lastName, pattern) ||
            substrFunc(r.street, pattern) || substrFunc(r.city, pattern) ||
            substrFunc(r.zipCode, pattern) )
        return true;
    // Checking numeric fields as well by treating them as strings:
    char numStr[MAX_NUM_STRING_SIZE];
    sprintf(numStr, "%ld", r.am);
    if ( substrFunc(numStr, pattern) ) return true;
    sprintf(numStr, "%d", r.streetNum);
    if ( substrFunc(numStr, pattern) ) return true;
    sprintf(numStr, "%f", r.salary);
    if ( substrFunc(numStr, pattern) ) return true;
    return false;
}

void printRecord(FILE *fp, Record r) {
    if (fp == NULL) fp = stdout;
    /* the following length specifiers in fprintf() are admittedly fitted to the
     * given inputs. Results may look less pretty for completely different inputs. */
    fprintf(fp, "%10ld | %15s | %18s | %15s | %6d | %18s | %7s | %9.2f\n", r.am,
            r.fisrtName, r.lastName, r.street, r.streetNum, r.city, r.zipCode, r.salary);
    /* fflush() is needed because even stdout is not automatically fflush'd on
     * newlines when redirected as we're doing in this program and we would
     * otherwise risk overflowing pipes! */
    fflush(fp);
}

/* Record List functions: */

RecordList *createRecordList(void) {
    RecordList *rList = malloc(sizeof(RecordList));
    if (rList == NULL) return NULL;
    rList->first = NULL;
    rList->last = NULL;
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
    newR->next = NULL;
    if (rList->first == NULL) {
        rList->first = rList->last = newR;
    } else {
        rList->last->next = newR;
        rList->last = rList->last->next;
    }
    return true;
}

void printRecordList(FILE *fp, RecordList *rList) {
    if (rList == NULL) return;
    if (fp == NULL) fp = stdout;
    RecordListNode *currentR = rList->first;
    while (currentR != NULL) {
        printRecord(fp, *(currentR->record));
        currentR = currentR->next;
    }
}

void prettyPrintResultSepLine(void) {
    printf("=========================================================================================================================\n");
}

void prettyPrintResultHeader(void) {
    printf("     AM    |    First Name   |      Last Name     |      Street     | St.Num |        City        | ZipCode |   Salary   \n");
    printf("-------------------------------------------------------------------------------------------------------------------------\n");
}
