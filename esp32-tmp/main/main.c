#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"

#define TAG "ESP32-TMP"

#define TMP_ADC_UNIT        (ADC_UNIT_1)
#define TMP_ADC_CHANNEL     (ADC_CHANNEL_7)
#define TMP_ADC_ATTEN       (ADC_ATTEN_DB_11)
#define TMP_ADC_BITWIDTH    (ADC_BITWIDTH_DEFAULT)

#define SCALE_TO_5V(VOLTAGE_3V3) ((float)VOLTAGE_3V3 * (5000.0 / 3300.0))
#define CONVERT_TO_TEMP(VOLTAGE) ((float)VOLTAGE / 10.0)

static adc_oneshot_unit_handle_t adc_handle;
static adc_cali_handle_t adc_cali_handle;
static int adc_raw_value;
static int adc_voltage;
static float adc_scaled_voltage;
static float adc_temperature;

void adc_init(void)
{
    adc_oneshot_unit_init_cfg_t adc_init_cfg = {
        .unit_id = TMP_ADC_UNIT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_init_cfg, &adc_handle));

    adc_oneshot_chan_cfg_t adc_channel_cfg = {
        .atten = TMP_ADC_ATTEN,
        .bitwidth = TMP_ADC_BITWIDTH,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, TMP_ADC_CHANNEL, &adc_channel_cfg));

    adc_cali_line_fitting_config_t adc_cali_cfg = {
        .unit_id = TMP_ADC_UNIT,
        .atten = TMP_ADC_ATTEN,
        .bitwidth = TMP_ADC_BITWIDTH,
    };
    ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&adc_cali_cfg, &adc_cali_handle));
}

void app_main(void)
{
    adc_init();

    while(1)
    {
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, TMP_ADC_CHANNEL, &adc_raw_value));
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc_cali_handle, adc_raw_value, &adc_voltage));
        adc_scaled_voltage = SCALE_TO_5V(adc_voltage);
        adc_temperature = CONVERT_TO_TEMP(adc_scaled_voltage);
        ESP_LOGI(TAG, "ADC raw value: %d, ADC voltage value: %f mV, Temperature: %f 'C", adc_raw_value, adc_scaled_voltage, adc_temperature);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
