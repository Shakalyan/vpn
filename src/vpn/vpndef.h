#ifndef VPNDEF_GUARD
#define VPNDEF_GUARD

#include <stdio.h>

#define TRUE 1
#define FALSE 0
typedef char bool;

#define FREE goto Error

#define WERR_FREE(TEMPLATE, ...) {          \
    fprintf(stderr, TEMPLATE, __VA_ARGS__); \
    FREE;                                   \
}

#define WERRS_FREE(MSG) {       \
    fprintf(stderr, "%s", MSG); \
    FREE;                       \
}

#endif