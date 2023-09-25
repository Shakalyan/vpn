#include <stdio.h>
#include "server/server.h"
#include "client/client.h"
#include <string.h>

#define MTU 2000

int main(int argc, char** argv) {

    if (argc != 2) {
        printf("Bad params\n");
        return 1;
    }
    if (strcmp(argv[1], "-s") == 0) {
        start_server("", "10.10.0.1", "255.255.255.0", 3535, MTU);
    } else if (strcmp(argv[1], "-c") == 0) {
        start_client("", "10.10.0.2", "255.255.255.0", "192.168.122.66", 3535, MTU);
    } else {
        printf("Bad params\n");
        return 1;
    }        

    return 0;
}