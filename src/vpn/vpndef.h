#ifndef VPNDEF_GUARD
#define VPNDEF_GUARD

#include <stdio.h>

#define TRUE 1
#define FALSE 0
typedef char bool;

#define FREE goto Error

#define WERR_FREE(TEMPLATE, ...) {          \
    fprintf(stdout, TEMPLATE, __VA_ARGS__); \
    FREE;                                   \
}

#define WERR_RET(RET, TEMPLATE, ...) {      \
    fprintf(stdout, TEMPLATE, __VA_ARGS__); \
    return RET;                             \
}

#define WERRS_FREE(MSG) {       \
    fprintf(stdout, "%s", MSG); \
    FREE;                       \
}

#endif