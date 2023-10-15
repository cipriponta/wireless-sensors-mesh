#include "socket_helpers.h"

int main()
{
    int server_file_descriptor, client_file_descriptor;
    int option = 1; 
    struct sockaddr_in server_socket_address;
    int server_socket_address_size = sizeof(server_socket_address);
    char received_msg[MSG_BUF_SIZE];
    char *sent_msg = "Hello from server";

    server_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if(server_file_descriptor < 0)
    {
        PRINT_ERROR_AND_EXIT("Could not create a new socket");
    }

    if(setsockopt(server_file_descriptor, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option)) < 0)
    {
        PRINT_ERROR_AND_EXIT("Could not modify the socket");
    }

    if(0 == set_server_address(&server_socket_address))
    {
         PRINT_ERROR_AND_EXIT("Could not set the ip address of the server");
    }

    if(bind(server_file_descriptor, (struct sockaddr*)&server_socket_address, server_socket_address_size) < 0)
    {
        PRINT_ERROR_AND_EXIT("Could not bind the socket");
    }
    
    if(listen(server_file_descriptor, MAX_NR_REQUEST) < 0)
    {
        PRINT_ERROR_AND_EXIT("Could not start the listen process");
    }

    printf("Waiting for a client to connect...\n");
    client_file_descriptor = accept(server_file_descriptor, (struct sockaddr*)&server_socket_address, (socklen_t*)&server_socket_address_size);
    if(client_file_descriptor < 0)
    {
        PRINT_ERROR_AND_EXIT("Could not connect to the client socket");
    }
    
    if(0 == receive_data(client_file_descriptor, received_msg, MSG_BUF_SIZE))
    {
        PRINT_ERROR_AND_EXIT("Could not receive data from client");
    }
    printf("Message from client: %s\n", received_msg);

    close(client_file_descriptor);
    shutdown(server_file_descriptor, SHUT_RDWR);

    return 0;
}