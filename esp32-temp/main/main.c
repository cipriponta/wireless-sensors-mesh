#include "esp_adc/adc_oneshot.h"

void app_main(void)
{
    // ADC1 Init
    adc_oneshot_unit_handle_t adc_handle;
    adc_oneshot_unit_init_cfg_t adc_init_cfg = 
    {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_init_cfg, &adc_handle));


    // ADC1 Config
    adc_oneshot_chan_cfg_t adc_chan_cfg = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_11,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ADC_CHANNEL_6, &adc_chan_cfg));
}
