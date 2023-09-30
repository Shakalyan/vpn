#include "config.h"
#include <stdio.h>
#include "../utils/json.h"
#include "../utils/hashmap.h"
#include <stdlib.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

#define DEFAULT_PORT "4242"
#define DEFAULT_IFNAME ""
#define DEFAULT_MTU "1500"
#define DEFAULT_NET_SETUP "./setup.sh"
#define DEFAULT_NET_CLEANUP "./cleanup.sh"

#define IF_NULL_EXIT(OBJ, MSG) {        \
    if (!OBJ) {                         \
        fprintf(stderr, "%s\n", MSG);   \
        goto Error;                     \
    }                                   \
}

#define IF_NOT_EXIT(OBJ, MSG) IF_NULL_EXIT(OBJ, MSG)

#define IF_NULL_SET_DEFAULT(OBJ, DEF) { \
    if (!OBJ) {                         \
        OBJ = DEF;                      \
    }                                   \
}

static int read_file(const char* path, char** out, size_t* size) 
{
    FILE* file = fopen(path, "r");
    if (!file) {
        perror("Open file error");
        return FALSE;
    }
    fseek(file, 0L, SEEK_END);
    *size = ftell(file);
    rewind(file);
    *out = malloc(1 + *size);
    (*out)[*size] = '\0';
    fread(*out, *size, 1, file);
    fclose(file);
    return TRUE;
}

server_conf_t* server_conf_alloc(size_t clients_size) 
{
    server_conf_t* conf = malloc(sizeof(server_conf_t));
    conf->clients = malloc(sizeof(sc_client_conf_t) * clients_size);
    return conf;
}

void server_conf_free(server_conf_t* conf) 
{
    free(conf->clients);
    free(conf);
}

int parse_server_conf(const char* path, server_conf_t* server_conf, network_conf_t* network_conf) 
{
    size_t json_size;
    char* json = NULL;
    hashmap_t* map = NULL;
    
    IF_NOT_EXIT(read_file(path, &json, &json_size), "Can't read config file");

    map = JSON_parse(json, json_size);
    IF_NULL_EXIT(map, "Config syntax is invalid");

    hashmap_t* server_obj = (hashmap_t*)hmap_get(map, "server");
    IF_NULL_EXIT(server_obj, "Server configuration is not present");
    char* port = (char*)hmap_get(server_obj, "port");
    IF_NULL_SET_DEFAULT(port, DEFAULT_PORT);
    char* prikey = (char*)hmap_get(server_obj, "prikey");
    IF_NULL_EXIT(prikey, "Server private key path is not present");

    hashmap_t* interface_obj = (hashmap_t*)hmap_get(server_obj, "interface");
    IF_NULL_EXIT(interface_obj, "Interface configuration is not present");
    char* ifname = (char*)hmap_get(interface_obj, "name");
    IF_NULL_SET_DEFAULT(ifname, DEFAULT_IFNAME);
    char* ifip = (char*)hmap_get(interface_obj, "ip");
    IF_NULL_EXIT(ifip, "Interface ip is not present");
    char* ifmask = (char*)hmap_get(interface_obj, "mask");
    IF_NULL_EXIT(ifmask, "Interface mask is not present");
    char* MTU = (char*)hmap_get(interface_obj, "MTU");
    IF_NULL_SET_DEFAULT(MTU, DEFAULT_MTU);
    
    obj_array_t* clients = (obj_array_t*)hmap_get(map, "clients");
    IF_NULL_EXIT(clients, "Clients configuration is not present");

    free(server_conf->clients);
    server_conf->clients = malloc(sizeof(sc_client_conf_t) * clients->size);
    for (int i = 0; i < clients->size; ++i) {
        char* ip = (char*)hmap_get(clients->objs[i], "ip");
        IF_NULL_EXIT(ip, "Clients must have an ip address");
        char* pubkey = (char*)hmap_get(clients->objs[i], "pubkey");
        IF_NULL_EXIT(pubkey, "Clients must have a pubkey");
        strcpy(server_conf->clients[i].ip, ip);
        strcpy(server_conf->clients[i].pubkey, pubkey);
    }
    server_conf->port = atoi(port);
    server_conf->interface.MTU = atoi(MTU);
    strcpy(server_conf->prikey, prikey);
    strcpy(server_conf->interface.name, ifname);
    strcpy(server_conf->interface.ip, ifip);
    strcpy(server_conf->interface.mask, ifmask);
    
    hashmap_t* network = (hashmap_t*)hmap_get(map, "network");
    if (network) {
        char* setup = (char*)hmap_get(network, "setup");
        IF_NULL_SET_DEFAULT(setup, DEFAULT_NET_SETUP);
        char* cleanup = (char*)hmap_get(network, "cleanup");
        IF_NULL_SET_DEFAULT(cleanup, DEFAULT_NET_CLEANUP);
        strcpy(network_conf->setup, setup);
        strcpy(network_conf->cleanup, cleanup);
    } else {
        strcpy(network_conf->setup, DEFAULT_NET_SETUP);
        strcpy(network_conf->cleanup, DEFAULT_NET_CLEANUP);
    }
    
    free(json);
    hmap_free(map);
    return TRUE;
    
Error:
    fprintf(stderr, "%s\n", get_ujson_err());
    free(json);
    hmap_free(map);
    return FALSE;
}

int parse_client_conf(const char* path, client_conf_t* client_conf, network_conf_t* network_conf) {
    size_t json_size;
    char* json = NULL;
    hashmap_t* map = NULL;
    
    IF_NOT_EXIT(read_file(path, &json, &json_size), "Can't read config file");
    map = JSON_parse(json, json_size);
    IF_NULL_EXIT(map, "Config syntax is invalid");

    hashmap_t* server_obj = (hashmap_t*)hmap_get(map, "server");
    IF_NULL_EXIT(server_obj, "Server configuration is not present");
    char* ip = (char*)hmap_get(server_obj, "ip");
    IF_NULL_EXIT(ip, "Server ip is not present");
    char* port = (char*)hmap_get(server_obj, "port");
    IF_NULL_SET_DEFAULT(port, DEFAULT_PORT);
    char* pubkey = (char*)hmap_get(server_obj, "pubkey");
    IF_NULL_EXIT(pubkey, "Server public key is not present");

    hashmap_t* client_obj = (hashmap_t*)hmap_get(map, "client");
    IF_NULL_EXIT(client_obj, "Client configuration is not present");
    char* prikey = (char*)hmap_get(client_obj, "prikey");
    IF_NULL_EXIT(prikey, "Client private key path is not present");

    hashmap_t* interface_obj = (hashmap_t*)hmap_get(client_obj, "interface");
    IF_NULL_EXIT(interface_obj, "Interface configuration is not present");
    char* ifname = (char*)hmap_get(interface_obj, "name");
    IF_NULL_SET_DEFAULT(ifname, DEFAULT_IFNAME);
    char *ifip = (char*)hmap_get(interface_obj, "ip");
    IF_NULL_EXIT(ifip, "Interface ip is not present");
    char *ifmask = (char*)hmap_get(interface_obj, "mask");
    IF_NULL_EXIT(ifmask, "Interface mask is not present");
    char* MTU = (char*)hmap_get(interface_obj, "MTU");
    IF_NULL_SET_DEFAULT(MTU, DEFAULT_MTU);

    strcpy(client_conf->server.ip, ip);
    strcpy(client_conf->server.pubkey, pubkey);
    client_conf->server.port = atoi(port);

    client_conf->interface.MTU = atoi(MTU);
    strcpy(client_conf->interface.name, ifname);
    strcpy(client_conf->interface.ip, ifip);
    strcpy(client_conf->interface.mask, ifmask);

    strcpy(client_conf->prikey, prikey);

    hashmap_t* network = (hashmap_t*)hmap_get(map, "network");
    if (network) {
        char* setup = (char*)hmap_get(network, "setup");
        IF_NULL_SET_DEFAULT(setup, DEFAULT_NET_SETUP);
        char* cleanup = (char*)hmap_get(network, "cleanup");
        IF_NULL_SET_DEFAULT(cleanup, DEFAULT_NET_CLEANUP);
        strcpy(network_conf->setup, setup);
        strcpy(network_conf->cleanup, cleanup);
    } else {
        strcpy(network_conf->setup, DEFAULT_NET_SETUP);
        strcpy(network_conf->cleanup, DEFAULT_NET_CLEANUP);
    }
    
    free(json);
    hmap_free(map);
    return TRUE;
    
Error:
    fprintf(stderr, "%s\n", get_ujson_err());
    free(json);
    hmap_free(map);
    return FALSE;
}