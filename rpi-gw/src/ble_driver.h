#ifndef __BLE_DRIVER_H__
#define __BLE_DRIVER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLE_COMMAND_MAX_SIZE            (128)
#define BLE_MSG_MAX_SIZE                (128)
#define BLE_MAC_ESP32_GAS_SENSOR        "78:E3:6D:19:43:1A"
#define BLE_MAC_ESP32_TMP_SENSOR        "A8:42:E3:48:32:2E"
#define BLE_SENSOR_VALUE_UUID           "0000FFCD"

void ble_get_sensor_value_char_handle(char *ble_mac, char *ble_char_uuid, int *handle);
void ble_get_sensor_value(char *ble_mac, int char_handle, int *sensor_value);

#endif