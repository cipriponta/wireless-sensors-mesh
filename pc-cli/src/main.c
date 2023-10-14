#include "socket_helpers.h"


int main()
{
    int client_file_descriptor;
    struct sockaddr_in server_address;
    char *sent_msg = "Hello from client";
    char received_msg[MSG_BUF];

    client_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if(client_file_descriptor < 0)
    {
        PRINT_ERROR_AND_EXIT("Could not create a new socket");
    }

    if(0 == set_server_address(&server_address))
    {
        PRINT_ERROR_AND_EXIT("Could not set the ip address of the server");
    }

    if(connect(client_file_descriptor, (struct sockaddr_in*)&server_address, sizeof(server_address)) < 0)
    {
        PRINT_ERROR_AND_EXIT("Could not connect to the server");
    }

    send(client_file_descriptor, sent_msg, sizeof(sent_msg), 0);
    printf("Message sent to server\n");
    read(client_file_descriptor, received_msg, sizeof(received_msg));
    printf("Message from server:%s\n", received_msg);

    return 0;
}