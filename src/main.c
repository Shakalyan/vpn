#include <stdio.h>
#include "vpn/server.h"
#include "vpn/client.h"
#include <string.h>
#include "utils/hashmap.h"
#include <stdlib.h>
#include "utils/json.h"
#include "vpn/config.h"
#include <unistd.h>
#include "vpn/args.h"

void start_as_server(vpn_args_t* args) {
    server_conf_t* server_conf = server_conf_alloc(0);
    network_conf_t* snetwork_conf = malloc(sizeof(network_conf_t));
    if (parse_server_conf(args->conf_path, server_conf, snetwork_conf)) {
        start_server(server_conf);
    } else {
        printf("S Config parse error\n");
    }
    server_conf_free(server_conf);
    free(snetwork_conf);
}

void start_as_client(vpn_args_t* args) {
    client_conf_t* client_conf = malloc(sizeof(client_conf_t));
    network_conf_t* cnetwork_conf = malloc(sizeof(network_conf_t));
    if (parse_client_conf(args->conf_path, client_conf, cnetwork_conf)) {
        start_client(client_conf);
    } else {
        printf("C Config parse error\n");
    }
    free(client_conf);
    free(cnetwork_conf);
}

int main(int argc, char** argv) 
{
    vpn_args_t args;
    if (!get_args(argc, argv, &args)) {
        return 1;
    }

    if (args.help) {
        printf("%s\n", get_help());
        return 0;
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