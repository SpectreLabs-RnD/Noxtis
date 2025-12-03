
#define _GNU_SOURCE

#include "config_local.h"

#include "algorithm_local.h"

#include "io_local.h"

#include <arpa/inet.h>

#include <errno.h>

#include <stdio.h>

#include <string.h>

#include <sys/epoll.h>

#include <sys/socket.h>

#include <sys/uio.h>

#include <unistd.h>


void start_noxtis_local(int local_fd, int remote_fd) {

    int ep = epoll_create1(0);

    if(ep < 0) {

        perror("epoll");

        return;

    }

    ep_add(ep, local_fd);

    ep_add(ep, remote_fd);

    struct epoll_event events[MAX_EVENTS];

    static uint8_t buf[BATCH_SIZE][BUF_SIZE];

    struct mmsghdr msgs[BATCH_SIZE];

    struct iovec iov[BATCH_SIZE];

    struct sockaddr_in addr[BATCH_SIZE];

    struct sockaddr_in client;

    socklen_t client_len = sizeof(client);

    int client_set = 0;

    for (int i = 0; i < BATCH_SIZE; i++) {

        iov[i].iov_base = buf[i];

        iov[i].iov_len = BUF_SIZE;

        memset(&msgs[i], 0, sizeof(msgs[i]));

        msgs[i].msg_hdr.msg_iov = &iov[i];

        msgs[i].msg_hdr.msg_iovlen = 1;

    }

    while (1) {

        int n = epoll_wait(ep, events, MAX_EVENTS, -1);
        
        if(n < 0) {
            
            continue;

        }

        for(int e = 0; e < n; e++) {

            int fd = events[e].data.fd;

            if(fd == local_fd) {

                for(int i = 0; i < BATCH_SIZE; i++) {
                
                    msgs[i].msg_hdr.msg_name = &addr[i];
                
                    msgs[i].msg_hdr.msg_namelen = sizeof(addr[i]);
                
                    msgs[i].msg_hdr.msg_flags = 0;
                
                }

                int r = recvmmsg(local_fd, msgs, BATCH_SIZE, 0, NULL);
                
                if(r <= 0) {
                    
                    continue;

                }

                for(int i = 0; i < r; i++) {
                    
                    size_t len = msgs[i].msg_len;
                    
                    if(len <= 0) {
                        
                        continue;

                    }

                    if(!client_set) {
                    
                        memcpy(&client, &addr[i], sizeof(client));
                    
                        client_set = 1;
                    
                    }

                    xor_data_fast(buf[i], len);
                    
                    send(remote_fd, buf[i], len, 0);
                
                }
            
            }

            else if(fd == remote_fd) {

                for(int i = 0; i < BATCH_SIZE; i++) {

                    msgs[i].msg_hdr.msg_name = NULL;

                }

                int r = recvmmsg(remote_fd, msgs, BATCH_SIZE, 0, NULL);
                
                if(r <= 0) {
                    
                    continue;

                }

                for(int i = 0; i < r; i++) {
                
                    size_t len = msgs[i].msg_len;
                
                    if(len <= 0) {
                        
                        continue;

                    }

                    xor_data_fast(buf[i], len);

                    if(client_set) {

                        sendto(local_fd, buf[i], len, 0, (struct sockaddr*)&client, client_len);
                    
                    }
                
                }
            
            }
        
        }
    
    }

}
