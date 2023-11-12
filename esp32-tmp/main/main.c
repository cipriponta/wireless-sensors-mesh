#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "ble_driver.h"
#include "adc_driver.h"

#include "sdkconfig.h"
             
#define TAG "ESP32-TMP"

void ble_response_cb(uint8_t *response_value, uint16_t *response_len);

adc_driver_config_t adc_config = {
    .handle = 0,
    .cali_handle = 0,
    .unit = ADC_UNIT_1,
    .channel = ADC_CHANNEL_7,
    .attenuation = ADC_ATTEN_DB_11,
    .bitwidth = ADC_BITWIDTH_DEFAULT,
};

adc_driver_value_t adc_value = {
    .raw_value = 0,
    .voltage = 0,
};

ble_driver_cfg_t ble_cfg = {
    .gatts_profile_id = 0x0000,
    .adv_service_uuid128 = {
        // Human readable form: 5B05F728-D088-41B7-852D-09027DA6383B             
        0x3B, 0x38, 0xA6, 0x7D, 0x02, 0x09, 0x2D, 0x85, 0xB7, 0x41, 0x88, 0xD0, 0x28, 0xF7, 0x05, 0x5B,
    },
    .device_name = "ESP32-TMP-SENSOR",
    .response_cb = ble_response_cb,
};

void app_main(void)
{
    ESP_LOGI(TAG, "Welcome");
    adc_init(&adc_config);
    ble_init(&ble_cfg);
}

void ble_response_cb(uint8_t *response_value, uint16_t* response_len)
{
    char buffer[ESP_GATT_MAX_ATTR_LEN];
    adc_read(&adc_config, &adc_value);
    sprintf(buffer, "%03d", adc_value.voltage);
    memcpy(response_value, buffer, ESP_GATT_MAX_ATTR_LEN * sizeof(char));
    *response_len = strlen(buffer);
}