#ifndef CONFIG_GUARD
#define CONFIG_GUARD

#include <stddef.h>

#define MAX_STR_SIZE 256

typedef struct interface_conf_t {
    char name[256];
    char ip[256];
    char mask[256];
    int MTU;
} interface_conf_t;

typedef struct network_conf_t {
    char setup[256];
    char cleanup[256];
} network_conf_t;


typedef struct sc_client_conf_t {
    char ip[256];
    char pubkey[256];
} sc_client_conf_t;

typedef struct server_conf_t {
    interface_conf_t interface;
    sc_client_conf_t* clients;
    char prikey[256];
    int port;
} server_conf_t;


typedef struct cc_server_conf_t {
    char ip[256];
    char pubkey[256];
    int port;
} cc_server_conf_t;

typedef struct client_conf_t {
    interface_conf_t interface;
    cc_server_conf_t server;
    char prikey[256];
} client_conf_t;


server_conf_t* server_conf_alloc(size_t clients_size);

int parse_server_conf(const char* path, server_conf_t* server_conf, network_conf_t* network_conf);

int parse_client_conf(const char* path, client_conf_t* client_conf, network_conf_t* network_conf);

void server_conf_free(server_conf_t* conf);

#endif