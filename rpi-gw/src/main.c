#include "socket_helpers.h"
#include "ble_driver.h"
#include <pigpio.h>

#define GAS_SENSOR_VALUE_THRESHOLD      (1000)
#define TMP_SENSOR_VALUE_THRESHOLD      (300)
#define FAN_GPIO_PIN                    (12)

typedef struct 
{
    int tmp_sensor_value;
    int gas_sensor_value;
    uint8_t fan_state;
}system_output_t;

int main()
{
    int server_file_descriptor, client_file_descriptor;
    int option = 1; 
    struct sockaddr_in server_socket_address;
    int server_socket_address_size = sizeof(server_socket_address);
    char received_msg[MSG_BUF_SIZE];

    int gas_sensor_value_handle = 0;
    int tmp_sensor_value_handle = 0;

    if(gpioInitialise() < 0)
    {
        PRINT_ERROR_AND_EXIT("Could not init GPIO");
    }
    gpioSetMode(FAN_GPIO_PIN, PI_OUTPUT);

    ble_get_sensor_value_char_handle(BLE_MAC_ESP32_GAS_SENSOR, BLE_SENSOR_VALUE_UUID, &gas_sensor_value_handle);
    printf("BLE Gas Sensor Handle: %d\n", gas_sensor_value_handle);
    ble_get_sensor_value_char_handle(BLE_MAC_ESP32_TMP_SENSOR, BLE_SENSOR_VALUE_UUID, &tmp_sensor_value_handle);
    printf("BLE Temperature Sensor Handle: %d\n", tmp_sensor_value_handle);

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
    
    while(1)
    {
        if(0 == receive_data(client_file_descriptor, received_msg, MSG_BUF_SIZE))
        {
            printf("Could not receive data from client\n");
        }

        if(0 == strcmp(received_msg, PING_COMMAND))
        {
            char sent_msg[MSG_BUF_SIZE];
            system_output_t system_output = {0};

            ble_get_sensor_value(BLE_MAC_ESP32_GAS_SENSOR, gas_sensor_value_handle, &system_output.gas_sensor_value);
            ble_get_sensor_value(BLE_MAC_ESP32_TMP_SENSOR, tmp_sensor_value_handle, &system_output.tmp_sensor_value);

            if(system_output.gas_sensor_value >= GAS_SENSOR_VALUE_THRESHOLD ||
               system_output.tmp_sensor_value >= TMP_SENSOR_VALUE_THRESHOLD)
            {
                system_output.fan_state = 1;
            }
            gpioPWM(FAN_GPIO_PIN, system_output.fan_state);

            sprintf(sent_msg, "Gas Value: %04d, Temperature Value: %d.%d'C, Fan State: %d", 
                    system_output.gas_sensor_value, 
                    system_output.tmp_sensor_value / 10, system_output.tmp_sensor_value % 10,
                    system_output.fan_state);

            if(send(client_file_descriptor, sent_msg, MSG_BUF_SIZE, 0) < 0)
            {
                printf("Could not send data to client\n");
            }
            else
            {
                printf("System stats sent to client successfully\n");
            }
        }
    }

    close(client_file_descriptor);
    shutdown(server_file_descriptor, SHUT_RDWR);
    gpioTerminate();
    return 0;
}
