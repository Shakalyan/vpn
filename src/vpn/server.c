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

#define CLIENTS_CNT 1


typedef struct client_id_t {
    in_addr_t saddr;
} client_id_t;

typedef struct client_t {
    client_id_t id;
    struct sockaddr_in addr;
} client_t;


static int setup_socket(int port) {
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        perror("Create server socket");
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    socklen_t server_addr_len = sizeof(server_addr);
    if (bind(socket_fd, (struct sockaddr*)&server_addr, server_addr_len) < 0) {
        perror("Bind server socket");
        return -1;
    }

    return socket_fd;
}

static void init_clients(client_t* clients) {
    clients[0].id.saddr = inet_addr("10.10.0.2");
}

static int set_client_info(client_t* clients, const struct iphdr* ip_header, const struct sockaddr_in* client_addr) {
    for (int i = 0; i < CLIENTS_CNT; ++i) {
        if (clients[i].id.saddr == ip_header->addrs.saddr) {
            memcpy(&clients[i].addr, client_addr, sizeof(struct sockaddr_in));
            return 1;
        }
    }
    return 0;
}

static client_t* find_client(client_t* clients, in_addr_t daddr) {
    for (int i = 0; i < CLIENTS_CNT; ++i) {
        if (clients[i].id.saddr == daddr)
            return &clients[i];
    }
    return NULL;
}

int start_server(const char* tun_name, const char* addr_str, const char* mask_str, int port, const int MTU) {

    int tun_fd = create_tun(tun_name, addr_str, mask_str);
    if (tun_fd < 0) {
        printf("Create tun error\n");
        return -1;
    }

    int socket_fd = setup_socket(port);
    if (socket_fd < 0) {
        printf("Create socket error\n");
        return -1;
    }

    fd_set read_fds;
    int max_fd = (socket_fd > tun_fd) ? socket_fd : tun_fd;

    client_t clients[CLIENTS_CNT];
    init_clients(clients);

    struct sockaddr_in client_addr;
    socklen_t client_addr_len;

    struct in_addr helper_addr;

    char buffer[MTU];
    while (1) {
        memset(&client_addr, 0, sizeof(client_addr));

        FD_ZERO(&read_fds);
        FD_SET(socket_fd, &read_fds);
        FD_SET(tun_fd, &read_fds);
        select(max_fd+1, &read_fds, NULL, NULL, NULL);

        if (FD_ISSET(socket_fd, &read_fds)) {
            int recv_cnt = recvfrom(socket_fd, buffer, MTU, 0, (struct sockaddr*)&client_addr, &client_addr_len);
            if (recv_cnt < 0) {
                perror("recvfrom socket");
                return -1;
            }

            struct iphdr* ip_header = (struct iphdr*)buffer;
            if (set_client_info(clients, ip_header, &client_addr)) {
                int write_cnt = write(tun_fd, buffer, recv_cnt);
                if (write_cnt < 0) {
                    perror("write to tun");
                    return -1;
                }
            } else {
                helper_addr.s_addr = ip_header->addrs.saddr;
                printf("Source IP (%s) doesn't match any client\n", inet_ntoa(helper_addr));
            }
        }

        if (FD_ISSET(tun_fd, &read_fds)) {
            int read_cnt = read(tun_fd, buffer, MTU);
            if (read_cnt < 0) {
                perror("read from tun");
                return -1;
            }

            struct iphdr* ip_header = (struct iphdr*)buffer;
            client_t* dclient = find_client(clients, ip_header->addrs.daddr);
            if (dclient != NULL) {
                int send_cnt = sendto(socket_fd, buffer, read_cnt, 0, (struct sockaddr*)&dclient->addr, sizeof(dclient->addr));
                if (send_cnt < 0) {
                    perror("sendto client socket");
                    return -1;
                }
            } else {
                helper_addr.s_addr = ip_header->addrs.saddr;
                printf("Destination IP (%s) doesn't match any client\n", inet_ntoa(helper_addr));
            }

        }

    }   


    return 0;
}