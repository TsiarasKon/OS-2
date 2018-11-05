#ifndef OS_2_STRUCTS_H
#define OS_2_STRUCTS_H

//typedef struct record {
//    int am;
//    char name[15];
//    char surname[25];
//    float salary;
//} Record;

typedef struct record {
    long am;
    char fisrtName[20];
    char lastName[20];
    char street[20];
    int streetNum;
    char city[20];
    char zipCode[20];
    float salary;
} Record;

typedef struct statistics {
    int runtime;
    int recordsNum;
} Statistics;

#endif
