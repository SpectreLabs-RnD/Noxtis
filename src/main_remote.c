
#include <stdio.h>

#include <string.h>

#include <unistd.h>

#include <stdlib.h>

#include <arpa/inet.h>

#include <sys/select.h>

#include "config_remote.h"

#include "algorithm_remote.h"

#include "io_remote.h"

#include "noxtis_remote.h"




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

	int local_port = LISTEN_PORT;

	char *remote_ip = TARGET_IP;

	int remote_port = TARGET_PORT;

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


    printf("[+] Server listening on %s:%d\n", local_ip, local_port);

    printf("[+] Forwarding decoded packets to %s:%d\n", remote_ip, remote_port);

    long int workers = sysconf(_SC_NPROCESSORS_ONLN);

    for(int i = 0; i < workers; i++) {

	pid_t pid = fork();

	if(pid < 0) {

		perror("fork failed");

		exit(0);

	}

	if(pid == 0) {

		break;

	}

    }

    int listen_sock = create_listen_socket(local_port);

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
    
    upstream_addr.sin_port = htons((uint16_t)local_port);

    if(inet_pton(AF_INET, remote_ip, &upstream_addr.sin_addr) != 1) {
    
        perror("inet_pton");
    
        close(listen_sock);
    
        close(upstream_sock);
    
        return 1;
    
    }

    unsigned char buffer[BUF_SIZE];

    make_nonblocking(listen_sock);
    
    make_nonblocking(upstream_sock);


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
