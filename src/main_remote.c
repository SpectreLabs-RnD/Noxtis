
#include <stdio.h>

#include <string.h>

#include <unistd.h>

#include <arpa/inet.h>

#include <sys/select.h>

#include "config_remote.h"

#include "algorithm_remote.h"

#include "io_remote.h"

#include "noxtis_remote.h"



int main(void) {


    int listen_sock = create_listen_socket(LISTEN_PORT);

    if(listen_sock < 0) {

        fprintf(stderr, "[-] Failed to create listening socket\n");

        return 1;

    }

    int upstream_sock = socket(AF_INET, SOCK_DGRAM, 0);

    if(upstream_sock < 0) {
    
        perror("socket");
    
        close(listen_sock);
    
        return 1;
    
    }

    struct sockaddr_in client_addr;
    
    socklen_t client_len = sizeof(client_addr);

    struct sockaddr_in upstream_addr;
    
    memset(&upstream_addr, 0, sizeof(upstream_addr));

    upstream_addr.sin_family = AF_INET;
    
    upstream_addr.sin_port = htons((uint16_t)TARGET_PORT);

    if(inet_pton(AF_INET, TARGET_IP, &upstream_addr.sin_addr) != 1) {
    
        perror("inet_pton");
    
        close(listen_sock);
    
        close(upstream_sock);
    
        return 1;
    
    }

    unsigned char buffer[BUF_SIZE];

    make_nonblocking(listen_sock);
    
    make_nonblocking(upstream_sock);

    printf("[+] Server listening on 0.0.0.0:%d\n", LISTEN_PORT);
    
    printf("[+] Forwarding decoded packets to %s:%d\n", TARGET_IP, TARGET_PORT);

    while (1) {

        fd_set fds;
    
        FD_ZERO(&fds);

        FD_SET(listen_sock, &fds);
    
        FD_SET(upstream_sock, &fds);

        int maxfd = (listen_sock > upstream_sock ? listen_sock : upstream_sock);

        int sel = select(maxfd + 1, &fds, NULL, NULL, NULL);
    
        if(sel < 0) {
        
            continue;
        
        }

        if(FD_ISSET(listen_sock, &fds)) {

            ssize_t r = recvfrom(listen_sock, buffer, (size_t)BUF_SIZE, 0, (struct sockaddr *)&client_addr, &client_len);

            if (r > 0) {

                size_t n = (size_t)r;

                xor_data_fast(buffer, n);

                (void)sendto(upstream_sock, buffer, n, 0, (struct sockaddr *)&upstream_addr, sizeof(upstream_addr));

            }

        }

        if(FD_ISSET(upstream_sock, &fds)) {

            ssize_t r = recvfrom(upstream_sock, buffer, (size_t)BUF_SIZE, 0, NULL, NULL);

            if(r > 0) {

                size_t n = (size_t)r;

                xor_data_fast(buffer, n);

                (void)sendto(listen_sock, buffer, n, 0, (struct sockaddr *)&client_addr, client_len);

            }

        }

    }

    close(listen_sock);
    
    close(upstream_sock);

    return 0;

}
