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
             
#define TAG "ESP32-GAS"

void ble_response_cb(uint8_t *response_value, uint16_t *response_len);

adc_driver_config_t adc_config = {
    .handle = 0,
    .cali_handle = 0,
    .unit = ADC_UNIT_2,
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
        // Human readable form: 86CBC57E-9993-4DE2-AE26-E2497E98EF7E
        0x7E, 0xEF, 0x98, 0x7E, 0x49, 0xE2, 0x26, 0xAE, 0xE2, 0x4D, 0x93, 0x99, 0x7E, 0xC5, 0xCB, 0x86,
    },
    .device_name = "ESP32-GAS-SENSOR",
    .response_cb = ble_response_cb,
};

void app_main(void)
{
    adc_init(&adc_config);
    ble_init(&ble_cfg);
}

void ble_response_cb(uint8_t *response_value, uint16_t* response_len)
{
    char buffer[ESP_GATT_MAX_ATTR_LEN];
    adc_read(&adc_config, &adc_value);
    sprintf(buffer, "%04d", adc_value.raw_value);
    memcpy(response_value, buffer, ESP_GATT_MAX_ATTR_LEN * sizeof(char));
    *response_len = strlen(buffer);
}