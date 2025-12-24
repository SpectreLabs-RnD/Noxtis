
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include "config_local.h"

#include "io_local.h"

#include "noxtis_local.h"

#include <unistd.h>



void print_help(const char *prog) {

    printf(
        " Usage: %s [arguments]\n"
        " \n"
        " Arguments:\n"
        " -h			Show this help and exit\n"
	" -la <local address>	Local IP address\n"
        " -lp <local port>	Local UDP port\n"
        " -ra <remote address>	Remote IP address\n"
        " -rp <remote port>	Remote UDP port\n"
        "\n",
        prog
    );

}


int main(int argc, char *argv[]) {

	char *local_ip = "0.0.0.0";

	int local_port = LOCAL_PORT;

	char *remote_ip = REMOTE_IP;

	int remote_port = REMOTE_PORT;

	for (int i = 1; i < argc; i++) {

                if(strcmp(argv[i], "-h") == 0) {

                        print_help(argv[0]);

                        return 0;

                }

		else if(strcmp(argv[i], "-lp") == 0) {

			if(i + 1 >= argc || argv[i + 1][0] == '-') {

				printf("-lp requires a port value\n");

				return 1;

			}

			local_port = atoi(argv[++i]);

		}

		else if (strcmp(argv[i], "-la") == 0) {

                        if(i + 1 >= argc || argv[i + 1][0] == '-') {

                                printf("-la requires an IP address value\n");

                                return 1;

                        }

			local_ip = argv[++i];


		}

		else if (strcmp(argv[i], "-ra") == 0) {

                        if(i + 1 >= argc || argv[i + 1][0] == '-') {

                                printf("-ra requires an IP address value\n");

                                return 1;

                        }

			remote_ip = argv[++i];

		}

		else if (strcmp(argv[i], "-rp") == 0) {

                        if(i + 1 >= argc || argv[i + 1][0] == '-') {

                                printf("-rp requires a port value\n");

                                return 1;

                        }

			remote_port = atoi(argv[++i]);

		}

		else {

			fprintf(stderr, "Unknown argument: %s\n", argv[i]);

			return 1;

		}

	}

    int local = udp_socket_bind(local_port, local_ip);

    if(local < 0) {
    
        perror("local bind");
    
        exit(1);
    
    }

    int remote = udp_connect_with_retries(remote_ip, remote_port, 10, 200);

    if(remote < 0) {
        
        perror("remote connect");
        
        exit(1);

    }

    set_sockbuf(local, 4 * 1024 * 1024);
    
    set_sockbuf(remote, 4 * 1024 * 1024);
    
    set_nonblock(local);
    
    set_nonblock(remote);

    printf("[+] UDP %d <-> %s:%d\n", local_port, remote_ip, remote_port);

    start_noxtis_local(local, remote);

    close(local);

    close(remote);

    return 0;

}
