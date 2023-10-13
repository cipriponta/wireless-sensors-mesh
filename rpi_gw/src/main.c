#include <stdio.h>
#include <sys/socket.h>

int main()
{
    int server_file_descriptor;
    int option = 1; 

    server_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if(server_file_descriptor < 0)
    {
        printf("Could not create a new socket\n");
    }

    if(0 == setsockopt(server_file_descriptor, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option)))
    {
        printf("Could not modify the socket");
    }

    return 0;
}