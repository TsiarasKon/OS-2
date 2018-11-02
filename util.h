#ifndef OS_2_UTIL_H
#define OS_2_UTIL_H

enum ErrorCodes {
    EC_OK,       // Success
    EC_ARG,      // Invalid command line arguments
    EC_FILE,     // Failed to open/read file
    EC_FORK,     // Error while forking
    EC_PIPE,     // Error related to pipes
    EC_MEM,      // Failed to allocate memory
    EC_UNKNOWN   // An unexpected error
};

#endif
