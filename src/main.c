#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "vpn/server.h"
#include "vpn/client.h"
#include "vpn/config.h"
#include "vpn/args.h"
#include "utils/system.h"
#include "crypto/AES.h"
#include "crypto/RSA.h"

#define FREE goto Error

#define WERR_FREE(TEMPLATE, ...) {          \
    fprintf(stderr, TEMPLATE, __VA_ARGS__); \
    FREE;                                   \
}

const char* net_cleanup_cmd = NULL;

void cleanup_network() 
{
    if (!net_cleanup_cmd) return;
    if (run_cmd(net_cleanup_cmd) != 0)
        fprintf(stderr, "Failed to cleanup network\n");
}

void handle_sigterm(int sig) 
{
    printf("\nTerminating...\nCleanup network...\n");
    cleanup_network();
    exit(0);
}

void start_as_server(vpn_args_t* args) 
{
    server_conf_t* server_conf = NULL;
    network_conf_t* network_conf = NULL;

    server_conf = server_conf_alloc(0);
    network_conf = network_conf_alloc();
    if (!parse_server_conf(args->conf_path, server_conf, network_conf)) 
        WERR_FREE("Failed to parse server config\n%s", "");
    
    net_cleanup_cmd = network_conf->cleanup;
    start_server(server_conf, network_conf);

    server_conf_free(server_conf);
    network_conf_free(network_conf);
    exit(0);

Error:
    server_conf_free(server_conf);
    network_conf_free(network_conf);
    exit(1);
}

void start_as_client(vpn_args_t* args) 
{
    client_conf_t* client_conf = NULL;
    network_conf_t* network_conf = NULL;

    client_conf = malloc(sizeof(client_conf_t));
    network_conf = malloc(sizeof(network_conf_t));
    if (!parse_client_conf(args->conf_path, client_conf, network_conf))
        WERR_FREE("Failed to parse client config\n%s", "");
    
    net_cleanup_cmd = network_conf->cleanup;    
    start_client(client_conf, network_conf);

    client_conf_free(client_conf);
    network_conf_free(network_conf);
    exit(0);

Error:
    client_conf_free(client_conf);
    network_conf_free(network_conf);
    exit(1);
}

int main(int argc, char** argv) 
{
    signal(SIGHUP, handle_sigterm);
    signal(SIGINT, handle_sigterm);
    signal(SIGQUIT, handle_sigterm);
    signal(SIGTERM, handle_sigterm);

    vpn_args_t args;
    if (!get_args(argc, argv, &args)) {
        return 1;
    }

    if (args.help) {
        printf("%s\n", get_help());
        return 0;
    }
    
    if (args.keys_dir_path) {
        return !generate_keys(args.keys_dir_path);
    }

    if (args.as_server) {
        start_as_server(&args);
        return 0;
    }

    if (args.as_client) {
        start_as_client(&args);
        return 0;
    }

    return 0;
}