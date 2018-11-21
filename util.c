#include "headers/util.h"

#include <unistd.h>
#include <sys/time.h>
#include <errno.h>

bool writeToFd(int fd, void *buffer, unsigned int bufSize) {
    size_t totalWritten = 0;
    ssize_t numWritten;
    while (true) {
        numWritten = write(fd, (char *) buffer + totalWritten, bufSize - totalWritten);
        if (numWritten < 0) {
            /* Added protection from signals just in case, eventhough
             * sigaction()'s SA_RESTART flag should be enough.
             * EINTR: interrupted by a signal before any data was written */
            if (errno == EINTR) continue;
            return false;
        }
        totalWritten += numWritten;
        if (totalWritten == bufSize) return true;
    }
}

bool readFromFd(int fd, void *buffer, unsigned int bufSize) {
    size_t totalRead = 0;
    ssize_t numRead;
    while (true) {
        numRead = read(fd, (char *) buffer + totalRead, bufSize - totalRead);
        if (numRead < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        totalRead += numRead;
        if (totalRead == bufSize) return true;
    }
}

long long getCurrentTime(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (((long long) tv.tv_sec) * 1000) + (tv.tv_usec / 1000);
}

int sumToN(int n) {      // known math formula for 1 + 2 + 3 + ... + n
    return (n * (n + 1)) / 2;
}

long min(long a, long b) {
    return (a > b) ? b : a;
}
