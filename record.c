#include <stdio.h>
#include "record.h"

void printRecord(Record r) {
    printf("%ld | %s %s | %s %d %s %s | %f\n", r.am, r.fisrtName, r.lastName,
            r.street, r.streetNum, r.city, r.zipCode, r.salary);
}
