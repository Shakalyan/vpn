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
#include "vpndef.h"

int create_tun(const char* name, const char* addr_str, const char* mask_str) {
    int tun_fd = -1, sock_fd = -1;

    tun_fd = open("/dev/net/tun", O_RDWR);
    EXIT_LOG_ERRNO_IF(tun_fd < 0, "Open /dev/net/tun");

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    memcpy(&ifr.ifr_ifrn.ifrn_name, name, strlen(name)+1);
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    EXIT_LOG_ERRNO_IF(ioctl(tun_fd, TUNSETIFF, (void*)&ifr) < 0, "ioctl TUNSETIFF");

    struct sockaddr_in addr, mask;
    addr.sin_family = AF_INET;
    mask.sin_family = AF_INET;
    EXIT_LOG_ERRNO_IF(inet_pton(AF_INET, addr_str, &addr.sin_addr) < 0, "Convert address to number");
    EXIT_LOG_ERRNO_IF(inet_pton(AF_INET, mask_str, &mask.sin_addr) < 0, "Convert mask to number");
    ifr.ifr_ifru.ifru_addr = *(struct sockaddr*)&addr;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    EXIT_LOG_ERRNO_IF(sock_fd < 0, "Create socket");

    EXIT_LOG_ERRNO_IF(ioctl(sock_fd, SIOCSIFADDR, (void*)&ifr) < 0, "ioctl SIOCSIFADDR");

    ifr.ifr_ifru.ifru_netmask = *(struct sockaddr*)&mask;
    EXIT_LOG_ERRNO_IF(ioctl(sock_fd, SIOCSIFNETMASK, (void*)&ifr) < 0, "ioctl SIOCSIFNETMASK");

    ifr.ifr_ifru.ifru_flags |= IFF_UP | IFF_RUNNING;
    EXIT_LOG_ERRNO_IF(ioctl(sock_fd, SIOCSIFFLAGS, (void*)&ifr) < 0, "ioctl SIOCSIFFLAGS");

    close(sock_fd);
    return tun_fd;

Error:
    close(tun_fd);
    close(sock_fd);
    return -1;
}