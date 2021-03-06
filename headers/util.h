#ifndef OS_2_UTIL_H
#define OS_2_UTIL_H

// assuming at most 64-bit numbers (2^64 can be stored in 21 chars in base 10)
#define MAX_NUM_STRING_SIZE 22

// For pipes:
#define READ_END 0
#define WRITE_END 1

#include <stdbool.h>

#define PRETTY_PRINT_RESULTS true
// BONUS feature: ignore case when searching pattern in Records
#define IGNORE_CASE false

enum ErrorCodes {
    EC_OK,       // Success
    EC_ARG,      // Invalid command line arguments
    EC_FILE,     // Failed to open/read file
    EC_FORK,     // Error while forking
    EC_EXEC,     // Failed to exec
    EC_PIPE,     // Error related to pipes
    EC_MEM,      // Failed to allocate memory
    EC_SIG,      // Signal related failure
    EC_INVALID,  // Invalid file format
    EC_UNKNOWN   // An unexpected error
};

bool writeToFd(int fd, void *buffer, unsigned int bufSize);
bool readFromFd(int fd, void *buffer, unsigned int bufSize);
long long getCurrentTime(void);
int sumToN(int n);
long min(long a, long b);

#endif
