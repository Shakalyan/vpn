#ifndef ARGS_GUARD
#define ARGS_GUARD

typedef struct vpn_args_t {
    char help;
    char as_client;
    char as_server;
    char* conf_path;
    char* keys_dir_path;
} vpn_args_t;

char get_args(int argc, char** argv, vpn_args_t* dst);

const char* get_help();

#endif