
#define _GNU_SOURCE
#include "noxtis_remote.h"
#include "config_remote.h"

#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <sys/epoll.h>

#include <sys/socket.h>

#include <sys/uio.h>

#include <unistd.h>




int create_listen_socket(int port) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return -1;
    }

       int opt = 1;

    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));  //) < 0) {

    //    perror("setsockopt SO_REUSEPORT failed");

      //  exit(0);

    //}

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((uint16_t)port);    // fixed

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(sock);
        return -1;
    }

    return sock;
}

int create_upstream_socket(const char *ip, int port) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);    // fixed

    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock);
        return -1;
    }

    // Optional: do NOT connect() if main code uses sendto()
    return sock;
}
