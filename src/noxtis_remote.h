
#pragma once

#ifndef NOXTIS_H

#define NOXTIS_H

#include <netinet/in.h>

int create_listen_socket(int port);

int create_upstream_socket(const char *ip, int port);

#endif
