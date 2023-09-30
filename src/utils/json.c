#include "json.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char ujson_err[256];

#define FREE goto Error

#define WERR_RET(RET, TEMPLATE, ...) {          \
    sprintf(ujson_err, TEMPLATE, __VA_ARGS__);   \
    return RET;                                 \
}

#define WERR_FREE(TEMPLATE, ...) {              \
    sprintf(ujson_err, TEMPLATE, __VA_ARGS__);   \
    FREE;                                       \
}

const char* get_ujson_err() {
    return ujson_err;
}

#define TRUE 1
#define FALSE 0
typedef char bool;


static bool skip_blank(const char *json, size_t size, size_t* offset) 
{
    static char blank[] = " \t\n";
    while (*offset < size) {
        bool is_blank = FALSE;
        for (char* b = blank; *b != '\0'; ++b) {
            if (json[*offset] == *b) {
                is_blank = TRUE;
                break;
            }
        }
        if (!is_blank) return TRUE;
        ++(*offset);
    }

    WERR_RET(FALSE, "Reached end of JSON while skipping blanks%s", "");
}

static bool retrieve_str(const char* json, size_t size, size_t* offset, char* dst) 
{
    char str_sym = json[*offset];
    ++(*offset);
    size_t from = *offset;

    while (*offset != size && str_sym != json[*offset]) 
        ++(*offset);

    if (*offset >= size)
        WERR_RET(FALSE, "Reached end of JSON while parsing string%s", "");

    size_t str_size = *offset - from;
    memcpy(dst, json + from, str_size);
    dst[str_size] = '\0';

    return TRUE;
}

static obj_array_t* objarr_alloc(size_t n) 
{
    obj_array_t* arr = malloc(sizeof(obj_array_t));
    arr->objs = malloc(sizeof(hashmap_t*) * n);
    arr->allocated = n;
    arr->size = 0;
    return arr;
}

static void add_to_objarr(obj_array_t* arr, hashmap_t* map) 
{
    if (arr->size == arr->allocated) {
        size_t allocated_size = sizeof(hashmap_t*) * arr->allocated;
        hashmap_t** objs = malloc(allocated_size * 2);
        memcpy(objs, arr->objs, allocated_size);
        free(arr->objs);
        arr->objs = objs;
    }
    arr->objs[arr->size++] = map;
}

static void objarr_free(obj_array_t* arr) 
{
    for (int i = 0; i < arr->size; ++i) {
        hmap_free(arr->objs[i]);
    }
    free(arr->objs);
    free(arr);
}

static hashmap_t* parse_object(const char* json, size_t size, size_t* offset);

static obj_array_t* parse_array(const char* json, size_t size, size_t* offset) 
{
    obj_array_t* objarr = objarr_alloc(4);
    while (*offset != size) {
        ++(*offset);
        if (!skip_blank(json, size, offset))
            FREE;
        
        if (json[*offset] != '{')
            WERR_FREE("'%c': not an object inside array at index %lu", json[*offset], *offset);
        
        hashmap_t* obj = parse_object(json, size, offset);
        if (!obj)
            FREE;

        add_to_objarr(objarr, obj);
        ++(*offset);

        if (!skip_blank(json, size, offset))
            FREE;

        if (json[*offset] == ',') continue;
        if (json[*offset] == ']') break;

        WERR_FREE("'%c': expected ',' or ']' at index %lu", json[*offset], *offset);
    }
    return objarr;

Error:
    objarr_free(objarr);
    return NULL;
}

static hashmap_t* parse_object(const char* json, size_t size, size_t* offset) 
{
    hashmap_t* obj = hmap_alloc(16, NULL);
    const size_t str_max_size = 256;
    char key[str_max_size], value_str[str_max_size];
    while (*offset != size) {
        ++(*offset);
        if (!skip_blank(json, size, offset))
            FREE;

        char curr_sym = json[*offset];
        if (curr_sym != '\'' && curr_sym != '"')
            WERR_FREE("'%c': expected a string name at index %lu", json[*offset], *offset);
        
        if (!retrieve_str(json, size, offset, key))
            FREE;
        
        ++(*offset);
        if (!skip_blank(json, size, offset))
            FREE;

        if (json[*offset] != ':')
            WERR_FREE("'%c': expected a ':' at index %lu", json[*offset], *offset);

        ++(*offset);
        if (!skip_blank(json, size, offset)) 
            FREE;

        if (json[*offset] == '{') {
            hashmap_t* value_obj = parse_object(json, size, offset);
            if (!value_obj) 
                FREE;
            hmap_put(obj, key, value_obj, sizeof(hashmap_t), (free_value_ptr_t)hmap_free, FALSE);
        } 
        else if (json[*offset] == '[') {
            obj_array_t* value_arr = parse_array(json, size, offset);
            if (!value_arr)
                FREE;
            hmap_put(obj, key, value_arr, sizeof(obj_array_t), (free_value_ptr_t)objarr_free, FALSE);
        } 
        else if (json[*offset] == '\'' || json[*offset] == '"') {
            if (!retrieve_str(json, size, offset, value_str))
                FREE;
            hmap_put(obj, key, value_str, str_max_size, free, TRUE);
        } 
        else 
            FREE;

        ++(*offset);
        if (!skip_blank(json, size, offset))
            FREE;

        if (json[*offset] == ',') continue;
        if (json[*offset] == '}') break;

        WERR_FREE("'%c': expected ',' or '}' at index %lu", json[*offset], *offset);
    }
    return obj;

Error:
    hmap_free(obj);
    return NULL;
}

hashmap_t* JSON_parse(const char *json, size_t size) 
{
    size_t offset = 0;
    skip_blank(json, size, &offset);
    if (json[offset] != '{') 
        WERR_RET(NULL, "'%c': expected '{' at index %lu", json[offset], offset);

    hashmap_t* result = parse_object(json, size, &offset);
    return result;
}