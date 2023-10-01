#include "server.h"
#include "tunnel.h"
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <linux/ip.h>
#include <unistd.h>
#include <stdlib.h>
#include "../utils/system.h"
#include "vpndef.h"
#include <errno.h>


typedef struct client_t {
    struct in_addr tun_addr;
    struct sockaddr_in act_addr;
} client_t;


static int setup_socket(int port) 
{
    int socket_fd = -1;

    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0)
        WERR_FREE("Failed to create socket: %s\n", strerror(errno))

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    socklen_t server_addr_len = sizeof(server_addr);
    if (bind(socket_fd, (struct sockaddr*)&server_addr, server_addr_len) < 0)
        WERR_FREE("Failed to bind server socket: %s\n", strerror(errno));

    return socket_fd;

Error:
    if (socket_fd != -1) close(socket_fd);
    return -1;
}

static bool init_clients(client_t* clients, sc_client_conf_t* clients_conf, size_t clients_size)
{
    for (size_t i = 0; i < clients_size; ++i) {
        if (!inet_aton(clients_conf->ip, &clients[i].tun_addr)) {
            fprintf(stderr, "Failed to transform address '%s' to number\n", clients_conf->ip);
            return FALSE;
        }
    }
    return TRUE;
}

static client_t* find_client(client_t* clients, size_t clients_size, in_addr_t daddr) 
{
    for (int i = 0; i < clients_size; ++i) {
        if (clients[i].tun_addr.s_addr == daddr)
            return &clients[i];
    }
    return NULL;
}

static bool set_client_act_addr(client_t* clients, size_t clients_size, const struct iphdr* ip_header, const struct sockaddr_in* client_addr) 
{
    for (size_t i = 0; i < clients_size; ++i) {
        if (clients[i].tun_addr.s_addr == ip_header->saddr) {
            memcpy(&clients[i].act_addr, client_addr, sizeof(struct sockaddr_in));
            return TRUE;
        }
    }
    return FALSE;
}


int start_server(server_conf_t* server_conf, network_conf_t* network_conf) 
{
    int tun_fd = -1, socket_fd = -1;
    char* buffer = NULL;
    client_t* clients = NULL;

    tun_fd = create_tun(server_conf->interface.name, server_conf->interface.ip, server_conf->interface.mask);
    if (tun_fd < 0)
        FREE;

    socket_fd = setup_socket(server_conf->port);
    if (socket_fd < 0)
        FREE;

    if (run_cmd(network_conf->setup) != 0)
        WERRS_FREE("Failed to setup network\n");

    size_t clients_size = server_conf->clients_size;
    clients = malloc(sizeof(client_t) * clients_size);
    if (!init_clients(clients, server_conf->clients, clients_size))
        FREE;

    struct sockaddr_in client_addr;
    socklen_t client_addr_len;

    struct in_addr helper_addr;

    fd_set read_fds;
    int max_fd = (socket_fd > tun_fd) ? socket_fd : tun_fd;
    buffer = malloc(server_conf->interface.MTU);
    while (1) {
        memset(&client_addr, 0, sizeof(client_addr));

        FD_ZERO(&read_fds);
        FD_SET(socket_fd, &read_fds);
        FD_SET(tun_fd, &read_fds);
        select(max_fd+1, &read_fds, NULL, NULL, NULL);

        if (FD_ISSET(socket_fd, &read_fds)) {
            int recv_cnt = recvfrom(socket_fd, buffer, server_conf->interface.MTU, 0, (struct sockaddr*)&client_addr, &client_addr_len);
            if (recv_cnt < 0)
                WERR_FREE("Failed to read from socket: %s\n", strerror(errno));

            struct iphdr* ip_header = (struct iphdr*)buffer;
            if (set_client_act_addr(clients, clients_size, ip_header, &client_addr)) {
                int write_cnt = write(tun_fd, buffer, recv_cnt);
                if (write_cnt < 0)
                    WERR_FREE("Failed to write to tunnel: %s\n", strerror(errno));
            } 
            else {
                helper_addr.s_addr = ip_header->addrs.saddr;
                printf("Source IP (%s) doesn't match any client\n", inet_ntoa(helper_addr));
            }
        }

        if (FD_ISSET(tun_fd, &read_fds)) {
            int read_cnt = read(tun_fd, buffer, server_conf->interface.MTU);
            if (read_cnt < 0)
                WERR_FREE("Failed to read from tunnel: %s\n", strerror(errno));

            struct iphdr* ip_header = (struct iphdr*)buffer;
            client_t* dclient = find_client(clients, clients_size, ip_header->addrs.daddr);
            if (dclient != NULL) {
                int send_cnt = sendto(socket_fd, buffer, read_cnt, 0, (struct sockaddr*)&dclient->act_addr, sizeof(dclient->act_addr));
                if (send_cnt < 0)
                    WERR_FREE("Failed to write to socket: %s\n", strerror(errno));
            } 
            else {
                helper_addr.s_addr = ip_header->addrs.saddr;
                printf("Destination IP (%s) doesn't match any client\n", inet_ntoa(helper_addr));
            }

        }

    }   

    run_cmd(network_conf->cleanup);
    close(tun_fd);
    close(socket_fd);
    free(buffer);
    free(clients);
    return 0;

Error:
    run_cmd(network_conf->cleanup);
    if (tun_fd != -1) close(tun_fd);
    if (socket_fd != -1) close(socket_fd);
    if (buffer) free(buffer);
    if (clients) free(clients);
    return 1;
}