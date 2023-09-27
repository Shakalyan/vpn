#include <stdio.h>
#include "vpn/server.h"
#include "vpn/client.h"
#include <string.h>
#include "utils/hashmap.h"
#include <stdlib.h>
#include "utils/json.h"
#include "vpn/config.h"

#define MTU 2000

int main(int argc, char** argv) {

    // if (argc != 2) {
    //     printf("Bad params\n");
    //     return 1;
    // }
    // if (strcmp(argv[1], "-s") == 0) {
    //     start_server("", "10.10.0.1", "255.255.255.0", 3535, MTU);
    // } else if (strcmp(argv[1], "-c") == 0) {
    //     start_client("", "10.10.0.2", "255.255.255.0", "192.168.122.66", 3535, MTU);
    // } else {
    //     printf("Bad params\n");
    //     return 1;
    // }

    server_conf_t* server_conf = server_conf_alloc(0);
    network_conf_t* snetwork_conf = malloc(sizeof(network_conf_t));
    if (!parse_server_conf("./server.json", server_conf, snetwork_conf)) {
        printf("Parsing error\n");
    } else {
        printf("Ok\n");
    }

    client_conf_t* client_conf = malloc(sizeof(client_conf_t));
    network_conf_t* cnetwork_conf = malloc(sizeof(network_conf_t));
    if (!parse_client_conf("./client.json", client_conf, cnetwork_conf)) {
        printf("Parsing client error\n");
    } else {
        printf("Ok\n");
    }


    server_conf_free(server_conf);
    free(snetwork_conf);
    free(client_conf);
    free(cnetwork_conf);



    return 0;
}