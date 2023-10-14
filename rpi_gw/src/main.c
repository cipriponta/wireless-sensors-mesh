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

#define SOCKET_PORT 8080
#define MAX_NR_REQUEST 1
#define MSG_BUF 128

int main()
{
    int server_file_descriptor, client_file_descriptor;
    int option = 1; 
    struct sockaddr_in socket_address;
    int socket_address_size = sizeof(socket_address);
    char received_msg[MSG_BUF];
    char *sent_msg = "Hello from RPI";

    server_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if(server_file_descriptor < 0)
    {
        PRINT_ERROR_AND_EXIT("Could not create a new socket");
    }

    if(setsockopt(server_file_descriptor, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option)) < 0)
    {
        PRINT_ERROR_AND_EXIT("Could not modify the socket");
    }

    socket_address.sin_family = AF_INET;
    socket_address.sin_addr.s_addr = INADDR_ANY;
    socket_address.sin_port = htons(SOCKET_PORT);

    if(bind(server_file_descriptor, (struct sockaddr*)&socket_address, socket_address_size) < 0)
    {
        PRINT_ERROR_AND_EXIT("Could not bind the socket");
    }

    if(listen(server_file_descriptor, MAX_NR_REQUEST) < 0)
    {
        PRINT_ERROR_AND_EXIT("Could not start the listen process");
    }

    client_file_descriptor = accept(server_file_descriptor, (struct sockaddr*)&socket_address, (socklen_t*)&socket_address_size);
    if(client_file_descriptor < 0)
    {
        PRINT_ERROR_AND_EXIT("Could not connect to the client socket");
    }

    send(client_file_descriptor, sent_msg, sizeof(sent_msg), 0);
    printf("Message sent to client\n");
    read(client_file_descriptor, received_msg, sizeof(received_msg)); 
    printf("Message from client: %s\n", received_msg);

    return 0;
}