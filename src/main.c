#include <stdio.h>
#include "vpn/server.h"
#include "vpn/client.h"
#include <string.h>
#include "utils/hashmap.h"
#include <stdlib.h>
#include "utils/json.h"
#include "vpn/config.h"
#include <unistd.h>

int run(char* script) {
    int inside_param = 0, params_cnt = 0, param_from = 0, param_to = 0;
    char param_sym = 0;
    for (char* s = script; *s != '\0'; ++s, ++param_to) {
        if (inside_param && *s == param_sym) {
            inside_param = 0;
        } else if (!inside_param && (*s == '\'' || *s == '"')) {
            inside_param = 1;
            param_sym = *s;
        }
        else if (!inside_param && *s == ' ') {
            if (param_to - param_from > 0)
                ++params_cnt;
            *s = '\0';
            param_from = param_to + 1;
        }
    }
    int idx = 0, str_len = param_to;
    char** args = malloc(sizeof(char*) * (params_cnt + 1));
    param_from = 0;
    for (int i = 0; i < str_len; ++i) {
        if (*(script + i) == '\0') {
            if (i - param_from > 0) {
                args[idx++] = script + param_from;
            }
            param_from = i+1;
        }
    }
    if (str_len - param_from > 0) {
        args[idx++] = script + param_from;
    }
    args[params_cnt] = NULL;
    int status = execv(args[0], args);
    free(args);

    return status;
}

int main(int argc, char** argv) {

    if (argc != 2) {
        printf("Bad params\n");
        return 1;
    }
    if (strcmp(argv[1], "-s") == 0) {
        server_conf_t* server_conf = server_conf_alloc(0);
        network_conf_t* snetwork_conf = malloc(sizeof(network_conf_t));
        if (parse_server_conf("./server.json", server_conf, snetwork_conf)) {
            start_server(server_conf);
        } else {
            printf("Parsing error\n");
        }
        server_conf_free(server_conf);
        free(snetwork_conf);
    } else if (strcmp(argv[1], "-c") == 0) {
        client_conf_t* client_conf = malloc(sizeof(client_conf_t));
        network_conf_t* cnetwork_conf = malloc(sizeof(network_conf_t));
        if (parse_client_conf("./client.json", client_conf, cnetwork_conf)) {
            start_client(client_conf);
        } else {
            printf("Parsing client error\n");
        }
        free(client_conf);
        free(cnetwork_conf);
    } else {
        printf("Bad params\n");
        return 1;
    }

    return 0;
}