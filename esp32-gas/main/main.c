#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "esp_bt.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gatt_common_api.h"

#include "sdkconfig.h"

#define TAG                                 "ESP32-GAS"
#define BLE_DEVICE_NAME                     "ESP32-GAS-SENSOR"

#define BLE_ADV_CONFIG_FLAG                 (1 << 0)
#define BLE_ADV_SCAN_RSP_CONFIG_FLAG        (1 << 1)
#define BLE_ADV_MIN_INTERVAL                (0x0020)
#define BLE_ADV_MAX_INTERVAL                (0x0040)
#define BLE_ADV_DATA_MIN_INTERVAL           (0x0006)
#define BLE_ADV_DATA_MAX_INTERVAL           (0x0010)

#define BLE_GATTS_PROFILE_ID                (0x0000)
#define BLE_GATTS_SERVICE_UUID              (0xFFCC)
#define BLE_GATTS_SERVICE_NUM_HANDLES       (0x0004)
#define BLE_GATTS_CHAR_UUID                 (0xFFCD)
#define BLE_GATTS_CHAR_MAX_LEN              (256 * sizeof(int))

#define GAS_ADC_UNIT                        (ADC_UNIT_2)
#define GAS_ADC_CHANNEL                     (ADC_CHANNEL_7)
#define GAS_ADC_ATTEN                       (ADC_ATTEN_DB_11)
#define GAS_ADC_BITWIDTH                    (ADC_BITWIDTH_DEFAULT)

#define SCALE_TO_5V(VOLTAGE_3V3) ((float)VOLTAGE_3V3 * (5000.0 / 3300.0))

struct gatts_profile_instance_t {
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t permissions;
    esp_gatt_char_prop_t property;
    uint16_t desc_handle;
    esp_bt_uuid_t desc_uuid;
};

void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
void ble_init(void);
void adc_init(void);

adc_oneshot_unit_handle_t adc_handle;
adc_cali_handle_t adc_cali_handle;
int adc_raw_value;
int adc_voltage;
float adc_scaled_voltage;

struct gatts_profile_instance_t gatts_profile_instance = {
    .gatts_cb = gatts_profile_event_handler,
    .gatts_if = ESP_GATT_IF_NONE,
};

uint8_t adv_service_uuid128[16] = {
    // Human readable form: 86CBC57E-9993-4DE2-AE26-E2497E98EF7E
    0x7E, 0xEF, 0x98, 0x7E, 0x49, 0xE2, 0x26, 0xAE, 0xE2, 0x4D, 0x93, 0x99, 0x7E, 0xC5, 0xCB, 0x86,
};

esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = true,
    .min_interval = BLE_ADV_DATA_MIN_INTERVAL,
    .max_interval = BLE_ADV_DATA_MAX_INTERVAL,
    .appearance = 0,
    .manufacturer_len = 0,
    .p_manufacturer_data = NULL,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(adv_service_uuid128),
    .p_service_uuid = adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

esp_ble_adv_data_t scan_rsp_data = {
    .set_scan_rsp = true,
    .include_name = true,
    .include_txpower = true,
    .appearance = 0,
    .manufacturer_len = 0, 
    .p_manufacturer_data =  NULL, 
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(adv_service_uuid128),
    .p_service_uuid = adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

esp_ble_adv_params_t adv_params = {
    .adv_int_min        = BLE_ADV_MIN_INTERVAL,
    .adv_int_max        = BLE_ADV_MAX_INTERVAL,
    .adv_type           = ADV_TYPE_IND,
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
    .channel_map        = ADV_CHNL_ALL,
    .adv_filter_policy  = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

uint8_t adv_config_done = 0;

uint8_t ble_reported_value[] = {'a', 'b', 'c'};

esp_attr_value_t gatts_char_value =
{
    .attr_max_len = BLE_GATTS_CHAR_MAX_LEN,
    .attr_len     = sizeof(ble_reported_value),
    .attr_value   = ble_reported_value,
};

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
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));
    ESP_ERROR_CHECK(esp_bluedroid_init());
    ESP_ERROR_CHECK(esp_bluedroid_enable());
    ESP_ERROR_CHECK(esp_ble_gatts_register_callback(gatts_event_handler));
    ESP_ERROR_CHECK(esp_ble_gap_register_callback(gap_event_handler));
    ESP_ERROR_CHECK(esp_ble_gatts_app_register(BLE_GATTS_PROFILE_ID));
    ESP_ERROR_CHECK(esp_ble_gatt_set_local_mtu(512));
}

void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch(event)
    {
        case ESP_GATTS_REG_EVT:
        {
            ESP_LOGI(TAG, "BLE Register App Event, status: %d, app_id: %d", param->reg.status, param->reg.app_id);

            gatts_profile_instance.service_id.is_primary = true;
            gatts_profile_instance.service_id.id.inst_id = 0;
            gatts_profile_instance.service_id.id.uuid.len = ESP_UUID_LEN_16;
            gatts_profile_instance.service_id.id.uuid.uuid.uuid16 = BLE_GATTS_SERVICE_UUID;

            ESP_ERROR_CHECK(esp_ble_gap_set_device_name(BLE_DEVICE_NAME));

            ESP_ERROR_CHECK(esp_ble_gap_config_adv_data(&adv_data));
            adv_config_done |= BLE_ADV_CONFIG_FLAG;
            ESP_ERROR_CHECK(esp_ble_gap_config_adv_data(&scan_rsp_data));
            adv_config_done |= BLE_ADV_SCAN_RSP_CONFIG_FLAG;

            ESP_ERROR_CHECK(esp_ble_gatts_create_service(gatts_if, &gatts_profile_instance.service_id, BLE_GATTS_SERVICE_NUM_HANDLES));

            break;
        }
        case ESP_GATTS_CREATE_EVT:
        {   
            ESP_LOGI(TAG, "Created service, status: %d, service_handle: %d", param->create.status, param->create.service_handle);
            gatts_profile_instance.service_handle = param->create.service_handle;
            gatts_profile_instance.char_uuid.len = ESP_UUID_LEN_16;
            gatts_profile_instance.char_uuid.uuid.uuid16 = BLE_GATTS_CHAR_UUID;
            ESP_ERROR_CHECK(esp_ble_gatts_start_service(gatts_profile_instance.service_handle));
            ESP_ERROR_CHECK(esp_ble_gatts_add_char(gatts_profile_instance.service_handle, 
                                                    &gatts_profile_instance.char_uuid, 
                                                    ESP_GATT_PERM_READ, 
                                                    ESP_GATT_CHAR_PROP_BIT_READ, 
                                                    &gatts_char_value,
                                                    NULL));
            break;
        }
        case ESP_GATTS_ADD_CHAR_EVT:
        {
            ESP_LOGI(TAG, "Created characteristic, status: %d, attribute_handle: %d, service_handle: %d", 
                        param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);
            gatts_profile_instance.char_handle = param->add_char.attr_handle;
            gatts_profile_instance.desc_uuid.len = ESP_UUID_LEN_16;
            gatts_profile_instance.desc_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;

            uint16_t recv_buff_length = 0;
            const uint8_t *recv_buff;
            ESP_ERROR_CHECK(esp_ble_gatts_get_attr_value(gatts_profile_instance.char_handle, &recv_buff_length, &recv_buff));
            ESP_LOGI(TAG, "Characteristic: ");
            ESP_LOG_BUFFER_CHAR(TAG, recv_buff, recv_buff_length);

            ESP_ERROR_CHECK(esp_ble_gatts_add_char_descr(gatts_profile_instance.service_handle, 
                                                         &gatts_profile_instance.desc_uuid,
                                                         ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                                                         NULL, 
                                                         NULL));
            break;
        }
        case ESP_GATTS_CONNECT_EVT:
        {
            esp_ble_conn_update_params_t conn_params = {0};
            memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
            conn_params.latency = 0;
            conn_params.max_int = 0x30;
            conn_params.min_int = 0x10;
            conn_params.timeout = 400;

            ESP_LOGI(TAG, "Connection event: conn_id: %d, remote: %02x:%02x:%02x:%02x:%02x:%02x",
                    param->connect.conn_id,  
                    param->connect.remote_bda[0],  
                    param->connect.remote_bda[1],  
                    param->connect.remote_bda[2],  
                    param->connect.remote_bda[3],  
                    param->connect.remote_bda[4],  
                    param->connect.remote_bda[5]);

            gatts_profile_instance.conn_id = param->connect.conn_id;
            ESP_ERROR_CHECK(esp_ble_gap_update_conn_params(&conn_params));
            break;
        }
        case ESP_GATTS_READ_EVT:
        {
            ESP_LOGI(TAG, "Read event: conn_id: %d, trans_id: %ld, handle: %d",  
                     param->read.conn_id, param->read.trans_id, param->read.handle); 

            esp_gatt_rsp_t response;
            memset(&response, 0, sizeof(esp_gatt_rsp_t));
            response.attr_value.handle = param->read.handle;
            response.attr_value.len = 3;
            response.attr_value.value[0] = 'a';
            response.attr_value.value[1] = 'b';
            response.attr_value.value[2] = 'c';

            ESP_ERROR_CHECK(esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id, ESP_GATT_OK, &response));
        }
            
        default:
            break;
    }
}

void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch(event)
    {
        case ESP_GATTS_REG_EVT:
            if(ESP_GATT_OK == param->reg.status)
            {
                gatts_profile_instance.gatts_if = gatts_if;
            }
            else
            {
                ESP_LOGI(TAG, "App registration failed, app_id: %x, status %d", param->reg.app_id, param->reg.status);
                return;
            }
            break;
        default:
            break;
    }

    if(ESP_GATT_IF_NONE == gatts_if || gatts_if == gatts_profile_instance.gatts_if)
    {
        if(gatts_profile_instance.gatts_cb)
        {
            gatts_profile_instance.gatts_cb(event, gatts_if, param);
        }
    }
}

void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch(event)
    {
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            adv_config_done &= (~BLE_ADV_CONFIG_FLAG);
            if (adv_config_done == 0)
            {
                esp_ble_gap_start_advertising(&adv_params);
            }
            break;
        case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
            adv_config_done &= (~BLE_ADV_SCAN_RSP_CONFIG_FLAG);
            if (adv_config_done == 0)
            {
                esp_ble_gap_start_advertising(&adv_params);
            }
            break;
        default:
            break;
    }
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
