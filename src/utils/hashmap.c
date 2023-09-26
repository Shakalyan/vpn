#include "hashmap.h"
#include <stdlib.h>
#include <string.h>

static uint64_t get_hash(char* key) {
    uint64_t hash = 0;
    for (int i = 1; *key != '\0'; ++i, ++key) {
        hash += *key * i;
    }
    return hash;
}

static inline void free_element_value(hashmap_t* map, hmap_el_t* el) {
    if (el->free_value) el->free_value(el->value);
    else if (map->free_value) map->free_value(el->value);
}

/**
 * @brief   You can pass free_value here for most elements and also pass it in hmap_put for specific.
 *          If free_value is NULL, then you must pass it every time you calling hmap_put function.
 */
hashmap_t* hmap_alloc(size_t modulus, free_value_ptr_t free_value) {
    hashmap_t* map = malloc(sizeof(hashmap_t));
    map->buckets = malloc(sizeof(hmap_bucket_t*) * modulus);
    memset(map->buckets, 0, sizeof(hmap_bucket_t*) * modulus);
    map->modulus = modulus;
    map->size = 0;
    map->free_value = free_value;
    return map;
}

void* hmap_get(hashmap_t* map, char* key) {
    size_t bidx = get_hash(key) % map->modulus;
    hmap_bucket_t* bucket = map->buckets[bidx];
    while (bucket) {
        if (strcmp(key, bucket->el.key) == 0) {
            return bucket->el.value;
        }
        bucket = bucket->next;
    }
    return NULL;
}

void hmap_put(hashmap_t* map, char* key, void* value, size_t value_size, free_value_ptr_t free_value) {
    size_t bidx = get_hash(key) % map->modulus;
    hmap_bucket_t** bucket = &map->buckets[bidx];
    while (*bucket) {
        if (strcmp(key, (*bucket)->el.key) == 0) {
            free_element_value(map, &(*bucket)->el);
            (*bucket)->el.value = malloc(value_size);
            memcpy((*bucket)->el.value, value, value_size);
            (*bucket)->el.free_value = free_value;
            return;
        }
        bucket = &(*bucket)->next;
    }
    *bucket = malloc(sizeof(hmap_bucket_t));

    size_t key_size = strlen(key)+1;
    (*bucket)->el.key = malloc(key_size);
    memcpy((*bucket)->el.key, key, key_size);

    (*bucket)->el.value = malloc(value_size);
    memcpy((*bucket)->el.value, value, value_size);

    (*bucket)->el.free_value = free_value;
    (*bucket)->next = NULL;
}

void hmap_free(hashmap_t* map) {
    hmap_bucket_t *current = NULL, *next = NULL;
    for (int i = 0; i < map->modulus; ++i) {
        current = map->buckets[i];
        while (current) {
            next = current->next;
            free(current->el.key);
            free_element_value(map, &current->el);
            free(current);
            current = next;
        }
    }
    free(map->buckets);
    free(map);
}