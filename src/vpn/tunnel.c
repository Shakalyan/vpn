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
#include <errno.h>

int create_tun(const char* name, const char* addr_str, const char* mask_str) 
{
    int tun_fd = -1, sock_fd = -1;

    tun_fd = open("/dev/net/tun", O_RDWR);
    if (tun_fd < 0)
        WERR_FREE("Failed to open /dev/net/tun: %s\n", strerror(errno));

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    memcpy(&ifr.ifr_ifrn.ifrn_name, name, strlen(name)+1);
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    if (ioctl(tun_fd, TUNSETIFF, (void*)&ifr) < 0)
        WERR_FREE("ioctl TUNSETIFF: %s\n", strerror(errno));

    struct sockaddr_in addr, mask;
    addr.sin_family = AF_INET;
    mask.sin_family = AF_INET;
    if (inet_pton(AF_INET, addr_str, &addr.sin_addr) < 0)
        WERRS_FREE("Failed to convert address to number\n")

    if (inet_pton(AF_INET, mask_str, &mask.sin_addr) < 0)
        WERRS_FREE("Failed to convert mask to number\n");

    ifr.ifr_ifru.ifru_addr = *(struct sockaddr*)&addr;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0)
        WERR_FREE("Failed to create socket: %s\n", strerror(errno));

    if (ioctl(sock_fd, SIOCSIFADDR, (void*)&ifr) < 0)
        WERR_FREE("ioctl SIOCSIFADDR: %s\n", strerror(errno));

    ifr.ifr_ifru.ifru_netmask = *(struct sockaddr*)&mask;
    if (ioctl(sock_fd, SIOCSIFNETMASK, (void*)&ifr) < 0)
        WERR_FREE("ioctl SIOCSIFNETMASK: %s\n", strerror(errno));

    ifr.ifr_ifru.ifru_flags |= IFF_UP | IFF_RUNNING;
    if (ioctl(sock_fd, SIOCSIFFLAGS, (void*)&ifr) < 0)
        WERR_FREE("ioctl SIOCSIFFLAGS: %s\n", strerror(errno));

    close(sock_fd);
    return tun_fd;

Error:
    if (tun_fd != -1) close(tun_fd);
    if (sock_fd != -1) close(sock_fd);
    return -1;
}