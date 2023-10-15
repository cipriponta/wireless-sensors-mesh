#ifndef __SOCKET_HELPERS_H__
#define __SOCKET_HELPERS_H__

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <unistd.h>

#define PRINT_ERROR_AND_EXIT(MSG)               \
do                                              \
{                                               \
    printf("%s\n", MSG);                        \
    exit(1);                                    \
} while (0)

#define SERVER_IP_ADDRESS   "192.168.0.194"
#define SOCKET_PORT         (8080)
#define MAX_NR_REQUEST      (1)
#define MSG_BUF             (128)

int set_server_address(struct sockaddr_in *server_socket_address);

#endif