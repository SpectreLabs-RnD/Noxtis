
#pragma once

#include <stdint.h>

#include <sys/epoll.h>

int udp_socket_bind(int port);

int udp_socket_connect(const char *ip, int port);

int udp_reconnect(int *fd, const char *ip, int port, int retry_delay_ms);

int udp_connect_with_retries(const char *ip, int port, int max_attempts, int retry_delay_ms);

int udp_socket_disconnect(int fd);

int set_nonblock(int fd);

int set_sockbuf(int fd, int size);

int ep_create();

int ep_add(int ep, int fd);
