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

hashmap_t* hmap_alloc(size_t modulus, free_value_ptr free_value) {
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

void hmap_put(hashmap_t* map, char* key, void* value, size_t value_size) {
    size_t bidx = get_hash(key) % map->modulus;
    hmap_bucket_t** bucket = &map->buckets[bidx];
    while (*bucket) {
        if (strcmp(key, (*bucket)->el.key) == 0) {
            map->free_value((*bucket)->el.value);
            (*bucket)->el.value = malloc(value_size);
            memcpy((*bucket)->el.value, value, value_size);
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

    (*bucket)->next = NULL;
}

void hmap_free(hashmap_t* map) {
    hmap_bucket_t *current = NULL, *next = NULL;
    for (int i = 0; i < map->modulus; ++i) {
        current = map->buckets[i];
        while (current) {
            next = current->next;
            free(current->el.key);
            map->free_value(current->el.value);
            free(current);
            current = next;
        }
    }
    free(map->buckets);
    free(map);
}