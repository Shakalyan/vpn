#ifndef JSON_GUARD
#define JSON_GUARD

#include "hashmap.h"

typedef struct obj_array_t {
    hashmap_t** objs;
    size_t size;
    size_t allocated;
} obj_array_t;


hashmap_t* JSON_parse(char *json, size_t size);

#endif