#include "args.h"
#include "../utils/system.h"
#include <string.h>
#include <stdio.h>

#define DEFAULT_CONF_PATH "./vpn.conf"

static const char help[] = \
"vpn [FLAGS|OPTIONS] [PARAMETERS]\n"
"FLAGS:\n"
"   -h --help   Print help information.\n"
"   -s --server Start as server.\n"
"   -c --client Start as client.\n"
"OPTIONS:\n"
"   -p --conf   Specify configuration file path."
;

static char* flags[] = {"-h --help",
                        "-c --client", 
                        "-s --server", NULL};

static char* prmtrs[] = {"-p --conf", NULL};


char get_args(int argc, char** argv, vpn_args_t* dst) {
    memset(dst, 0, sizeof(vpn_args_t));
    dst->conf_path = DEFAULT_CONF_PATH;
    if (!parse_args(argc, argv, flags, prmtrs, dst, sizeof(vpn_args_t))) {
        printf("%s\n", get_usys_err());
        return 0;
    }
    return 1;
}

const char* get_help() {
    return help;
}