#include "tunnel.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <string.h>
#include <arpa/inet.h>
#include <linux/ip.h>

int create_tun(const char* name, const char* addr_str, const char* mask_str) {

    int tun_fd = open("/dev/net/tun", O_RDWR);
    if (tun_fd < 0) {
        perror("Open /dev/net/tun");
        return -1;
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    memcpy(&ifr.ifr_ifrn.ifrn_name, name, strlen(name)+1);
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

    if (ioctl(tun_fd, TUNSETIFF, (void*)&ifr)) {
        perror("Ioctl TUNSETIFF");
        close(tun_fd);
        return -1;
    }

    struct sockaddr_in addr, mask;
    addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, addr_str, &addr.sin_addr) < 0) {
        perror("INET PTON");
        return -1;
    }
    mask.sin_family = AF_INET;
    if (inet_pton(AF_INET, mask_str, &mask.sin_addr) < 0) {
        perror("INET PTON");
        return -1;
    }
    ifr.ifr_ifru.ifru_addr = *(struct sockaddr*)&addr;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    if (ioctl(sockfd, SIOCSIFADDR, (void*)&ifr)) {
        perror("Ioctl SIOCSIFADDR");
        close(tun_fd);
        close(sockfd);
        return -1;
    }

    ifr.ifr_ifru.ifru_netmask = *(struct sockaddr*)&mask;
    if (ioctl(sockfd, SIOCSIFNETMASK, (void*)&ifr)) {
        perror("Ioctl SIOCSIFNETMASK");
        close(tun_fd);
        close(sockfd);
        return -1;
    }

    ifr.ifr_ifru.ifru_flags |= IFF_UP | IFF_RUNNING;
    if (ioctl(sockfd, SIOCSIFFLAGS, (void*)&ifr)) {
        perror("Ioctl SIOCSIFFLAGS");
        close(tun_fd);
        close(sockfd);
        return -1;
    }

    return tun_fd;
}