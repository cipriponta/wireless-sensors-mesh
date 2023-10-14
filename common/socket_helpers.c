#include "socket_helpers.h"

int set_server_address(struct sockaddr_in *server_socket_address)
{
    server_socket_address->sin_family = AF_INET;
    server_socket_address->sin_port = htons(SOCKET_PORT);
    if(0 == inet_pton(AF_INET, SERVER_IP_ADDRESS, &server_socket_address->sin_addr.s_addr))
    {
        return 0;
    }
    return 1;
}

