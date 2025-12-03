
#include "io_local.h"

#include <arpa/inet.h>

#include <fcntl.h>

#include <stdio.h>

#include <string.h>

#include <unistd.h>

#include <sys/socket.h>



int udp_socket_bind(int port) {
    
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    
    if (fd < 0) {
        
        return -1;

    }

    struct sockaddr_in addr = {0};
    
    addr.sin_family = AF_INET;
    
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    addr.sin_port = htons((uint16_t)port);

    if(bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {

        close(fd);
        
        return -1;
    
    }
    
    return fd;

}

int udp_socket_connect(const char *ip, int port) {

    int fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (fd < 0) {
        
        return -1;

    }

    struct sockaddr_in addr = {0};

    addr.sin_family = AF_INET;
    
    addr.sin_port = htons((uint16_t)port);
    
    inet_pton(AF_INET, ip, &addr.sin_addr);

    if(connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {

        close(fd);
        
        return -1;
    
    }
    
    return fd;

}

int set_nonblock(int fd) {

    int flags = fcntl(fd, F_GETFL, 0);

    if(flags < 0) {
        
        return -1;
    
    }
    
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);

}

int set_sockbuf(int fd, int size) {
    
    if(setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &size, sizeof size) < 0) {
        
        return -1;
    
    }

    if(setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &size, sizeof size) < 0) {
        
        return -1;
    
    }

    return 0;

}

int ep_create() {

    return epoll_create1(0);

}

int ep_add(int ep, int fd) {

    struct epoll_event ev = { .events = EPOLLIN, .data.fd = fd };

    return epoll_ctl(ep, EPOLL_CTL_ADD, fd, &ev);

}
