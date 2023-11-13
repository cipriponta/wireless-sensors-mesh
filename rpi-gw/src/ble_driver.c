#include "ble_driver.h"

static void get_linux_cmd_output(char *comm, char *output, int output_len);

void ble_get_sensor_value_char_handle(char *ble_mac, char *ble_char_uuid, int *handle)
{
    char command[BLE_COMMAND_MAX_SIZE];
    char output[BLE_MSG_MAX_SIZE];
    size_t buff_size = 0;

    sprintf(command, "sudo gatttool -b %s --char-desc | awk -F\",|= |-\" '{print $2, $4}' | grep -i %s | awk '{print $1}'", ble_mac, ble_char_uuid);
    get_linux_cmd_output(command, output, BLE_MSG_MAX_SIZE);

    buff_size = strlen(output) - 1;
    if('\n' == output[buff_size])
    {
        output[buff_size] = '\0';
    }

    *handle = strtol(output, NULL, 16);
}

void ble_get_sensor_value(char *ble_mac, int char_handle, int *sensor_value)
{
    char command[BLE_COMMAND_MAX_SIZE];
    char output[BLE_MSG_MAX_SIZE];

    sprintf(command, "sudo gatttool -b %s --char-read --handle=0x%04x | awk -F \": \" '{print $2}' | awk '{for(i=1;i<=NF;i++) printf(\"%%d\", $i)}'", ble_mac, char_handle);
    get_linux_cmd_output(command, output, BLE_MSG_MAX_SIZE);

    *sensor_value = 0;
    size_t output_len = strlen(output);
    for(size_t chr = 0; chr < output_len / 2; chr++)
    {
        char temp_buffer[3];
        strncpy(temp_buffer, output + chr * 2, 2);
        temp_buffer[2] = '\0';
        int value = strtol(temp_buffer, NULL, 0);
        *sensor_value = *sensor_value * 10 + strtol(temp_buffer, NULL, 0) - 30;
    }
}

void get_linux_cmd_output(char *comm, char *output, int output_len)
{
    FILE *pipe_file;
    size_t pipe_file_len;

    pipe_file = popen(comm, "r");
    if(NULL == pipe_file)
    {
        printf("Pipe file generated by the command is NULL\n");
        return;
    }

    fgets(output, output_len, pipe_file);

    pclose(pipe_file);
}