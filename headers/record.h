#ifndef OS_2_RECORD_H
#define OS_2_RECORD_H

#include <stdbool.h>

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
void printRecord(Record r);

#endif