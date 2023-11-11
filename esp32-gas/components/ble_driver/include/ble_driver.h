#ifndef __BLE_DRIVER_H__
#define __BLE_DRIVER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "nvs_flash.h"
#include "esp_log.h"

#include "esp_bt.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gatt_common_api.h"

#define BLE_LOG_TAG                         "BLE_DRIVER"
#define BLE_ADV_CONFIG_FLAG                 (1 << 0)
#define BLE_ADV_SCAN_RSP_CONFIG_FLAG        (1 << 1)
#define BLE_GATTS_CHAR_MAX_LEN              (256 * sizeof(int))

typedef struct
{
    uint8_t adv_service_uuid128[16];
    uint16_t gatts_profile_id;
    char *device_name;
    void (*response_cb)(uint8_t *response_value, uint16_t *response_len);
} ble_driver_cfg_t;

void ble_init(ble_driver_cfg_t *ble_cfg);
#endif