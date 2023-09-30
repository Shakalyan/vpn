#include "system.h"
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>
#include "hashmap.h"

char usys_err[256];

#define FREE goto Error

#define WERR_RET(RET, TEMPLATE, ...) {          \
    sprintf(usys_err, TEMPLATE, __VA_ARGS__);   \
    return RET;                                 \
}

#define WERR_FREE(TEMPLATE, ...) {              \
    sprintf(usys_err, TEMPLATE, __VA_ARGS__);   \
    FREE;                                       \
}

const char* get_usys_err() {
    return usys_err;
}

#define TRUE 1
#define FALSE 0
typedef char bool;


static int run_cmd_forked(const char* cmd) 
{
    size_t cmd_size = strlen(cmd) + 1;
    char* cmd_cpy = malloc(cmd_size);
    memcpy(cmd_cpy, cmd, cmd_size);

    int in_arg = FALSE; 
    int argc = 0, from = 0;
    char arg_qt = 0;
    for (int i = 0; i < cmd_size; ++i) {
        if (in_arg && cmd_cpy[i] == arg_qt) {
            in_arg = FALSE;
        } 
        else if (!in_arg && (cmd_cpy[i] == '\'' || cmd_cpy[i] == '"')) {
            in_arg = TRUE;
            arg_qt = cmd_cpy[i];
        }
        else if (!in_arg && (cmd_cpy[i] == ' ' || cmd_cpy[i] == '\0')) {
            if (i - from > 0)
                ++argc;
            cmd_cpy[i] = '\0';
            from = i + 1;
        }
    }

    char** args = malloc(sizeof(char*) * (argc + 1));
    int arg_idx = 0;
    from = 0;
    for (int i = 0; i < cmd_size; ++i) {
        if (cmd_cpy[i] == '\0') {
            if (i - from > 0) {
                args[arg_idx++] = cmd_cpy + from;
            }
            from = i+1;
        }
    }

    args[argc] = NULL;
    int status = execvp(args[0], args);
    free(args);
    free(cmd_cpy);

    return status;
}

int run_cmd(const char* cmd) 
{
    int pid = fork();
    if (pid == 0) {
        int status = run_cmd_forked(cmd);
        if (status < 0) {
            fprintf(stderr, "%s: %s\n", cmd, strerror(errno));
            exit(1);
        }
        exit(0);
    }
    
    int status;
    waitpid(pid, &status, 0);

    return WEXITSTATUS(status);
}


#define VALID_BRD_SYM(SYM) ((SYM >= 'a' && SYM <= 'z') || (SYM >= 'A' && SYM <= 'Z') || (SYM >= '0' && SYM <= '9'))
#define VALID_CNT_SYM(SYM) (VALID_BRD_SYM(SYM) || SYM == '-' || SYM == '_')

static const int FLAG_SIZE = sizeof(char);
static const int PRMTR_SIZE = sizeof(char*);
static const int DWORD_SIZE = sizeof(void*);


static bool retrieve_arg_names(char* name, char** full, char* shrt) 
{
    int name_len = strlen(name);

    if (name_len < 7 || name[0] != '-' || name[2] != ' ' || name[3] != '-' || name[4] != '-')
        return FALSE;
    
    if (!(VALID_BRD_SYM(name[1]) && VALID_BRD_SYM(name[5]) && VALID_BRD_SYM(name[name_len-1])))
        return FALSE;
    
    for (int i = 6; i < name_len-1; ++i) {
        if (!VALID_CNT_SYM(name[i]))
            return FALSE;
    }

    *full = name + 3;
    *shrt = name[1];
    return TRUE;
}

static bool map_names_to_sbyte(char** names, hashmap_t* fn_sbyte_map, char* sn_sbyte_arr, int* sbyte, int ofs) 
{
    while (*names) {
        char* full;
        char shrt;
        if (retrieve_arg_names(*names, &full, &shrt)) {
            hmap_put(fn_sbyte_map, full, sbyte, sizeof(int), NULL, TRUE);
            sn_sbyte_arr[(int)shrt] = *sbyte;
        } else
            WERR_RET(FALSE, "Bad arg names syntax '%s'. Example: '-s --full'", *names);

        ++names;
        *sbyte += ofs;
    }
    return TRUE;
}

static bool process_arg_names(char** flags, char** prmtrs, size_t dst_size,
                              hashmap_t** fn_sbyte_map, char* sn_sbyte_arr, 
                              int* flags_cnt, int* prmtrs_cnt)
{
    int fcnt = 0, pcnt = 0;
    while (*flags++) ++fcnt;
    while (*prmtrs++) ++pcnt;

    flags -= fcnt+1;
    prmtrs -= pcnt+1;
    *fn_sbyte_map = hmap_alloc(fcnt + pcnt, free);
    *flags_cnt = fcnt;
    *prmtrs_cnt = pcnt;
    
    int sbyte = 0;
    if (!map_names_to_sbyte(flags, *fn_sbyte_map, sn_sbyte_arr, &sbyte, FLAG_SIZE))
        return FALSE;
    
    sbyte += DWORD_SIZE - (fcnt * FLAG_SIZE % DWORD_SIZE);

    if (!map_names_to_sbyte(prmtrs, *fn_sbyte_map, sn_sbyte_arr, &sbyte, PRMTR_SIZE))
        return FALSE;
    
    if (sbyte > dst_size)
        WERR_RET(FALSE, "Destination structure is too small (%lu bytes). Need at least %d bytes", dst_size, sbyte);

    return TRUE;
}

static bool check_arg(char* arg, int* arg_len, bool* is_short) 
{
    int alen = strlen(arg);

    if (alen < 2 || arg[0] != '-')
        WERR_RET(FALSE, "Bad arg '%s'", arg);
    
    if (arg[1] != '-') {
        for (int i = 1; i < alen; ++i) {
            if (!VALID_BRD_SYM(arg[i]))
                WERR_RET(FALSE, "Invalid symbol in arg '%s' at pos %d", arg, i+1);
        }
        *is_short = TRUE;        
    } 
    else {
        if (alen < 4 || !(VALID_BRD_SYM(arg[2]) && VALID_BRD_SYM(arg[alen-1])))
            WERR_RET(FALSE, "Bad arg '%s'", arg);
    
        for (int i = 0; i < alen-1; ++i) {
            if (!VALID_CNT_SYM(arg[i]))
                WERR_RET(FALSE, "Invalid symbol in arg '%s' at pos %d", arg, i+1);
        }
        *is_short = FALSE;
    }
    
    *arg_len = alen;

    return TRUE;
}

static bool find_short(char s, char* sn_sbyte_arr, int flags_cnt, int* sbyte, bool* is_flag) {
    if (sn_sbyte_arr[(int)s] == -1)
        return FALSE;
    *sbyte = sn_sbyte_arr[(int)s];
    *is_flag = *sbyte < flags_cnt * FLAG_SIZE;
    return TRUE;
}

static bool parse_short(char** argv, int* arg_idx, void* dst, int arg_len, char* sn_sbyte_arr, int flags_cnt) 
{
    char* arg = argv[*arg_idx];
    bool prmtr_present = FALSE;

    for (int i = 1; i < arg_len; ++i) {
        bool is_flag;
        int sbyte;
        if (!find_short(arg[i], sn_sbyte_arr, flags_cnt, &sbyte, &is_flag))
            WERR_RET(FALSE, "Unknown arg '%s'", argv[*arg_idx]);
        
        if (is_flag) {
            memset(dst + sbyte, TRUE, FLAG_SIZE);
        } else {
            if (prmtr_present)
                WERR_RET(FALSE, "Too many parametrized args in '%s'", argv[*arg_idx]);
            if (!argv[*arg_idx+1])
                WERR_RET(FALSE, "No parameter for parametrized arg in '%s'", argv[*arg_idx]);
            
            memcpy(dst + sbyte, &argv[*arg_idx+1], PRMTR_SIZE);
            prmtr_present = TRUE;
        }
    }

    if (prmtr_present)
        *arg_idx += 1;
    
    return TRUE;
}

static bool parse_full(char** argv, int* arg_idx, void* dst, int arg_len, hashmap_t* fn_sbyte_map, int flags_cnt) {
    int* sbyte = (int*)hmap_get(fn_sbyte_map, argv[*arg_idx]);
    if (!sbyte)
        WERR_RET(FALSE, "Unknown arg '%s'", argv[*arg_idx]);
    
    if (*sbyte < flags_cnt * FLAG_SIZE) {
        memset(dst + *sbyte, TRUE, FLAG_SIZE);
    } else {
        if (!argv[*arg_idx+1])
            WERR_RET(FALSE, "No parameter for parametrized arg in '%s'", argv[*arg_idx]);

        memcpy(dst + *sbyte, &argv[*arg_idx+1], PRMTR_SIZE);
        *arg_idx += 1;
    }

    return TRUE;
}

bool parse_args(int argc, char** argv, 
                char** flags, char** prmtrs, 
                void* dst, size_t dst_size) 
{
    hashmap_t* fn_sbyte_map = NULL;
    char sn_sbyte_arr[128];
    memset(sn_sbyte_arr, -1, 128);

    int flags_cnt;
    int prmtrs_cnt;
    if (!process_arg_names(flags, prmtrs, dst_size, &fn_sbyte_map, sn_sbyte_arr, &flags_cnt, &prmtrs_cnt))
        FREE;
    
    for (int i = 1; i < argc; ++i) {
        int arg_len;
        bool is_short;
        if (!check_arg(argv[i], &arg_len, &is_short))
            FREE;

        if (is_short) {
            if (!parse_short(argv, &i, dst, arg_len, sn_sbyte_arr, flags_cnt))
                FREE;
        } else {
            if (!parse_full(argv, &i, dst, arg_len, fn_sbyte_map, flags_cnt))
                FREE;
        }            
    }

    hmap_free(fn_sbyte_map);
    return TRUE;

Error:
    hmap_free(fn_sbyte_map);
    return FALSE;
}