#ifndef OS_2_UTIL_H
#define OS_2_UTIL_H

// assuming at most 64-bit numbers (2^64 can be stored in 21 chars in base 10)
#define MAX_NUM_STRING_SIZE 22

// For pipes:
#define WRITE_END 1
#define READ_END 0

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

#endif
