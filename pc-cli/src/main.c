#include "socket_helpers.h"

int main()
{
    int client_file_descriptor;
    struct sockaddr_in server_address;
    char *sent_msg = "Hello from client";
    char system_stats[MSG_BUF_SIZE];

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


    while(1)
    {
        if(send(client_file_descriptor, PING_COMMAND, MSG_BUF_SIZE, 0) < 0)
        {
            printf("Could not send data to client\n");
        }
        else
        {
            if(0 == receive_data(client_file_descriptor, system_stats, MSG_BUF_SIZE))
            {
                printf("Could not receive data from client\n");
            }
            else
            {
                printf("System stats: %s\n", system_stats);
            }
        }
        
        sleep(1);
    }
    
    shutdown(client_file_descriptor, SHUT_RDWR);
    return 0;
}