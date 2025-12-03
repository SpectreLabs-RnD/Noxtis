
#pragma once

#include <stdint.h>

#include <sys/epoll.h>

int udp_socket_bind(int port);

int udp_socket_connect(const char *ip, int port);

int set_nonblock(int fd);

int set_sockbuf(int fd, int size);

int ep_create();

int ep_add(int ep, int fd);
