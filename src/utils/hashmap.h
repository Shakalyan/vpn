#ifndef HASHMAP_GUARD
#define HASHMAP_GUARD

#include <stddef.h>
#include <stdint.h>

typedef void (*free_value_ptr_t)(void*);

typedef struct hmap_el_t {
    char* key;
    void* value;
    free_value_ptr_t free_value;
} hmap_el_t;

typedef struct hmap_bucket_t {
    hmap_el_t el;
    struct hmap_bucket_t* next;
} hmap_bucket_t;

typedef struct hashmap_t {
    size_t size;
    size_t modulus;
    free_value_ptr_t free_value;
    hmap_bucket_t** buckets;
} hashmap_t;


hashmap_t* hmap_alloc(size_t modulus, free_value_ptr_t free_value);

void* hmap_get(hashmap_t* map, char* key);

void hmap_put(hashmap_t* map, char* key, void* value, size_t value_size, free_value_ptr_t free_value);

void hmap_free(hashmap_t* map);


#endif