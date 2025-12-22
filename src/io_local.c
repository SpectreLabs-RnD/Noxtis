#define _GNU_SOURCE

#define _POSIX_C_SOURCE 199309L

#include "io_local.h"

#include <arpa/inet.h>

#include <fcntl.h>

#include <errno.h>

#include <time.h>

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

int udp_reconnect(int *fd, const char *ip, int port, int retry_delay_ms) {

	if(*fd < 0) {

		*fd = udp_socket_connect(ip, port);

		if(*fd >= 0) {

			return 0;

		}

	}

	struct sockaddr_in addr;

	memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;

	addr.sin_port = htons((uint16_t)port);

	inet_pton(AF_INET, ip, &addr.sin_addr);

	while(1) {

        	/* Flush pinned tuple before reconnecting */
        	udp_socket_disconnect(*fd);

        	if(connect(*fd, (struct sockaddr *)&addr, sizeof(addr)) == 0) {

			return 0;

		}

		struct timespec ts;

		ts.tv_sec  = retry_delay_ms / 1000;

		ts.tv_nsec = (retry_delay_ms % 1000) * 1000000;

		nanosleep(&ts, NULL);

	}

}




int udp_connect_with_retries(const char *ip, int port, int max_attempts, int retry_delay_ms) {

	int fd = -1;

	for(int i = 0; i < max_attempts; i++) {


		fd = udp_socket_connect(ip, port);

		if(fd >= 0) {

			return fd;

		}

		struct timespec ts;

		ts.tv_sec  = retry_delay_ms / 1000;

		ts.tv_nsec = (retry_delay_ms % 1000) * 1000000;

		nanosleep(&ts, NULL);

	}

	return -1;
}

int udp_socket_disconnect(int fd) {

	struct sockaddr sa;

	memset(&sa, 0, sizeof(sa));

	sa.sa_family = AF_UNSPEC;

	return connect(fd, &sa, sizeof(sa));

}
