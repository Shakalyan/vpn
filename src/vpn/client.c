#include "client.h"
#include "tunnel.h"
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <linux/ip.h>
#include <stdlib.h>

int start_client(client_conf_t* conf) 
{

    int tun_fd = create_tun(conf->interface.name, conf->interface.ip, conf->interface.mask);
    if (tun_fd < 0) {
        printf("Create tun error\n");
        return -1;
    }

    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        perror("Create socket");
        return -1;
    }
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(conf->server.port);
    server_addr.sin_addr.s_addr = inet_addr(conf->server.ip);
    socklen_t server_addr_len = sizeof(server_addr);
    
    fd_set read_fds;
    char* buffer = malloc(conf->interface.MTU);
    int max_fd = (socket_fd > tun_fd) ? socket_fd : tun_fd;
    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(socket_fd, &read_fds);
        FD_SET(tun_fd, &read_fds);
        select(max_fd+1, &read_fds, NULL, NULL, NULL);

        if (FD_ISSET(tun_fd, &read_fds)) {
            int cnt = read(tun_fd, buffer, conf->interface.MTU);
            if (cnt < 0) {
                perror("read from tun");
                return -1;
            }
            if (cnt == 0) {
                printf("Read from tun EOF\n");
                return -1;
            }
            cnt = sendto(socket_fd, buffer, cnt, 0, (struct sockaddr*)&server_addr, server_addr_len);
            if (cnt < 0) {
                perror("sendto socket");
                return -1;
            }
        }
        if (FD_ISSET(socket_fd, &read_fds)) {
            int cnt = read(socket_fd, buffer, conf->interface.MTU);
            if (cnt < 0) {
                perror("read from socket");
                return -1;
            }
            if (cnt == 0) {
                printf("Read from socket EOF\n");
                return -1;
            }
            cnt = write(tun_fd, buffer, cnt);
            if (cnt < 0) {
                perror("write to tun");
                return -1;
            }
        }
    }

    return 0;
}