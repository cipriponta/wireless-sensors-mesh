#ifndef __ADC_DRIVER_H__
#define __ADC_DRIVER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#define SCALE_TO_5V(VOLTAGE_3V3) ((float)VOLTAGE_3V3 * (5000.0 / 3300.0))

typedef struct
{
    adc_oneshot_unit_handle_t handle;
    adc_cali_handle_t cali_handle;
    adc_unit_t unit;
    adc_channel_t channel;
    adc_atten_t attenuation;
    adc_bitwidth_t bitwidth;
} adc_driver_config_t;

typedef struct
{
    int raw_value;
    int voltage;
} adc_driver_value_t;

void adc_init(adc_driver_config_t *adc_cfg);
void adc_read(adc_driver_config_t *adc_cfg, adc_driver_value_t *adc_value);

#endif