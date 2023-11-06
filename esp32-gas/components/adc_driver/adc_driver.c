#include "adc_driver.h"

void adc_init(adc_driver_config_t *adc_cfg)
{
    adc_oneshot_unit_init_cfg_t adc_init_cfg = {
        .unit_id = adc_cfg->unit,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_init_cfg, &adc_cfg->handle));

    adc_oneshot_chan_cfg_t adc_channel_cfg = {
        .atten = adc_cfg->attenuation,
        .bitwidth = adc_cfg->bitwidth,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_cfg->handle, adc_cfg->channel, &adc_channel_cfg));

    adc_cali_line_fitting_config_t adc_cali_cfg = {
        .unit_id = adc_cfg->unit,
        .atten = adc_cfg->attenuation,
        .bitwidth = adc_cfg->bitwidth,
    };
    ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&adc_cali_cfg, &adc_cfg->cali_handle));
}

void adc_read(adc_driver_config_t *adc_cfg, adc_driver_value_t *adc_value)
{
    int unscaled_voltage;
    ESP_ERROR_CHECK(adc_oneshot_read(adc_cfg->handle, adc_cfg->channel, &adc_value->raw_value));
    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc_cfg->cali_handle, adc_value->raw_value, &unscaled_voltage));
    adc_value->voltage = SCALE_TO_5V(unscaled_voltage);
}