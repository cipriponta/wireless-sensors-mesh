#ifndef __SOCKET_HELPERS_H__
#define __SOCKET_HELPERS_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <unistd.h>

#define PRINT_ERROR_AND_EXIT(MSG)               \
do                                              \
{                                               \
    printf("%s\n", MSG);                        \
    exit(1);                                    \
} while (0)

#define SERVER_IP_ADDRESS           "192.168.1.199"
#define SOCKET_PORT                 (8080)
#define MAX_NR_REQUEST              (1)
#define CHUNK_BUF_SIZE              (128)
#define MSG_BUF_SIZE                (128)
#define PING_COMMAND                "ping"

#define PING_COMMAND        "ping"

int set_server_address(struct sockaddr_in *server_socket_address);
int receive_data(int socket_file_descriptor, char *buffer, int buffer_max_size);

#endif