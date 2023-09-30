#ifndef SYSTEM_GUARD
#define SYSTEM_GUARD

#include <stddef.h>

int run_cmd(const char* cmd);

char parse_args(int argc, char** argv, 
               char** flags, char** prmtrs, 
               void* dst, size_t dst_size);

const char* get_usys_err();

#endif