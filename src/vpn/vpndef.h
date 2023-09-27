#ifndef VPNDEF_GUARD
#define VPNDEF_GUARD

#include <stdio.h>

#define ERROR_FREE_MARK Error

#define EXIT_LOG_IF(CONDITION, MESSAGE) {   \
    if (CONDITION) {                        \
        fprintf(stderr, "%s\n", MESSAGE);   \
        goto ERROR_FREE_MARK;               \
    }                                       \
}

#define EXIT_LOG_ERRNO_IF(CONDITION, MESSAGE) { \
    if (CONDITION) {                            \
        perror(MESSAGE);                        \
        goto ERROR_FREE_MARK;                   \
    }                                           \
}

#endif