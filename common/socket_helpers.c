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

int receive_data(int socket_file_descriptor, char *buffer, int buffer_max_size)
{
    if(NULL == buffer)
    {
        printf("The buffer used for receiving data is null\n");
        return 0;
    }

    char chunk[CHUNK_BUF_SIZE];
    int chunk_size_recv = 0;
    int buffer_curr_size = 0;

    do
    {
        strcpy(chunk, "");
        chunk_size_recv = recv(socket_file_descriptor, chunk, sizeof(chunk), 0);

        // printf("Chunk size recv: %d, chunk: %s\n", chunk_size_recv, chunk);

        if(chunk_size_recv < 0)
        {
            printf("Could not receive data properly\n");
            return 0;
        }
        else if(0 == chunk_size_recv)
        {
            break;
        }

        for(int i = 0; i < chunk_size_recv; i++)
        {
            if(buffer_curr_size + i >= buffer_max_size)
            {
                printf("The received message overflows the buffer\n");
                return 0;
            }

            buffer[buffer_curr_size + i] = chunk[i];

            if(chunk[i] == '\0')
            {
                return 1;
            }
        }
        buffer_curr_size += chunk_size_recv;
    } while(chunk_size_recv > 0);

    printf("Could not find the null terminator\n");
    return 0;
}