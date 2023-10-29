#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gatt_common_api.h"

#define TAG "ESP32-GAS"

#define BLE_GATTS_PROFILE_ID 0

#define GAS_ADC_UNIT        (ADC_UNIT_2)
#define GAS_ADC_CHANNEL     (ADC_CHANNEL_7)
#define GAS_ADC_ATTEN       (ADC_ATTEN_DB_11)
#define GAS_ADC_BITWIDTH    (ADC_BITWIDTH_DEFAULT)

#define SCALE_TO_5V(VOLTAGE_3V3) ((float)VOLTAGE_3V3 * (5000.0 / 3300.0))

static adc_oneshot_unit_handle_t adc_handle;
static adc_cali_handle_t adc_cali_handle;
static int adc_raw_value;
static int adc_voltage;
static float adc_scaled_voltage;

void gatts_event_handler(void);
void gap_event_handler(void);
void ble_init(void);
void adc_init(void);

void app_main(void)
{
    ble_init();
    adc_init();

    while(1)
    {
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, GAS_ADC_CHANNEL, &adc_raw_value));
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc_cali_handle, adc_raw_value, &adc_voltage));
        adc_scaled_voltage = SCALE_TO_5V(adc_voltage);
        ESP_LOGI(TAG, "ADC raw value: %d, ADC voltage value: %f mV", adc_raw_value, adc_scaled_voltage);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void ble_init(void)
{
    esp_err_t returned_error;

    returned_error = nvs_flash_init();
    if(ESP_ERR_NVS_NO_FREE_PAGES == returned_error || ESP_ERR_NVS_NEW_VERSION_FOUND == returned_error)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        returned_error = nvs_flash_init();
    }
    ESP_ERROR_CHECK(returned_error);

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));

    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));

    ESP_ERROR_CHECK(esp_bluedroid_init());

    // ESP_ERROR_CHECK(esp_ble_gatts_register_callback(gatts_event_handler));

    // ESP_ERROR_CHECK(esp_ble_gap_register_callback(gap_event_handler));

    // ESP_ERROR_CHECK(esp_ble_gatts_app_register(BLE_GATTS_PROFILE_ID));

    // ESP_ERROR_CHECK(esp_ble_gatt_set_local_mtu(512));
}

void gatts_event_handler(void)
{

}

void gap_event_handler(void)
{

}

void adc_init(void)
{
    adc_oneshot_unit_init_cfg_t adc_init_cfg = {
        .unit_id = GAS_ADC_UNIT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_init_cfg, &adc_handle));

    adc_oneshot_chan_cfg_t adc_channel_cfg = {
        .atten = GAS_ADC_ATTEN,
        .bitwidth = GAS_ADC_BITWIDTH,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, GAS_ADC_CHANNEL, &adc_channel_cfg));

    adc_cali_line_fitting_config_t adc_cali_cfg = {
        .unit_id = GAS_ADC_UNIT,
        .atten = GAS_ADC_ATTEN,
        .bitwidth = GAS_ADC_BITWIDTH,
    };
    ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&adc_cali_cfg, &adc_cali_handle));
}
