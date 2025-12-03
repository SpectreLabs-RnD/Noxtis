
#include <stdio.h>

#include <stdlib.h>

#include "config_local.h"

#include "io_local.h"

#include "noxtis_local.h"

#include <unistd.h>


int main() {

    int local = udp_socket_bind(LOCAL_PORT);

    if(local < 0) {
    
        perror("local bind");
    
        exit(1);
    
    }

    int remote = udp_socket_connect(REMOTE_IP, REMOTE_PORT);
    
    if(remote < 0) {
        
        perror("remote connect");
        
        exit(1);

    }

    set_sockbuf(local, 4 * 1024 * 1024);
    
    set_sockbuf(remote, 4 * 1024 * 1024);
    
    set_nonblock(local);
    
    set_nonblock(remote);

    printf("[+] UDP %d <-> %s:%d\n", LOCAL_PORT, REMOTE_IP, REMOTE_PORT);

    start_noxtis_local(local, remote);

    close(local);

    close(remote);

    return 0;

}
