#ifndef CONFIG_GUARD
#define CONFIG_GUARD

#include <stddef.h>

#define MAX_STR_SIZE 256

typedef struct interface_conf_t {
    char name[MAX_STR_SIZE];
    char ip[MAX_STR_SIZE];
    char mask[MAX_STR_SIZE];
    int MTU;
} interface_conf_t;

typedef struct network_conf_t {
    char setup[MAX_STR_SIZE];
    char cleanup[MAX_STR_SIZE];
} network_conf_t;


typedef struct sc_client_conf_t {
    char ip[MAX_STR_SIZE];
    char pubkey[MAX_STR_SIZE];
} sc_client_conf_t;

typedef struct server_conf_t {
    interface_conf_t interface;
    sc_client_conf_t* clients;
    size_t clients_size;
    char prikey[MAX_STR_SIZE];
    int port;
} server_conf_t;


typedef struct cc_server_conf_t {
    char ip[MAX_STR_SIZE];
    char pubkey[MAX_STR_SIZE];
    int port;
} cc_server_conf_t;

typedef struct client_conf_t {
    interface_conf_t interface;
    cc_server_conf_t server;
    char prikey[MAX_STR_SIZE];
} client_conf_t;


int parse_server_conf(const char* path, server_conf_t* server_conf, network_conf_t* network_conf);

int parse_client_conf(const char* path, client_conf_t* client_conf, network_conf_t* network_conf);

server_conf_t* server_conf_alloc(size_t clients_size);
void server_conf_free(server_conf_t* conf);

client_conf_t* client_conf_alloc();
void client_conf_free(client_conf_t* conf);

network_conf_t* network_conf_alloc();
void network_conf_free(network_conf_t* conf);

#endif