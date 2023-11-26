#include "ble_driver.h"

typedef struct
{
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint16_t service_num_handles;
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
} gatts_profile_instance_t;

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
static void gatts_profile_sensor_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

static ble_driver_cfg_t ble_cfg_instance;

static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = true,
    .min_interval = 0x0006,
    .max_interval = 0x0010,
    .appearance = 0,
    .manufacturer_len = 0,
    .p_manufacturer_data = NULL,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = 0,
    .p_service_uuid = NULL,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static esp_ble_adv_data_t scan_rsp_data = {
    .set_scan_rsp = true,
    .include_name = true,
    .include_txpower = true,
    .appearance = 0,
    .manufacturer_len = 0, 
    .p_manufacturer_data =  NULL, 
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = 0,
    .p_service_uuid = NULL,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static esp_ble_adv_params_t adv_params = {
    .adv_int_min        = 0x0020,
    .adv_int_max        = 0x0040,
    .adv_type           = ADV_TYPE_IND,
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
    .channel_map        = ADV_CHNL_ALL,
    .adv_filter_policy  = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

static uint8_t adv_config_done = 0;

static gatts_profile_instance_t gatts_profile_instance = {
    .gatts_if = ESP_GATT_IF_NONE,
    .gatts_cb = gatts_profile_sensor_event_handler,
    .service_id = {
        .is_primary = true,
        .id = {
            .inst_id = 0,
            .uuid = {
                .len = ESP_UUID_LEN_16,
                .uuid.uuid16 = 0xFFCC,
            }
        }
    },
    .service_num_handles = 4,
    .char_uuid = {
        .len = ESP_UUID_LEN_16,
        .uuid.uuid16 = 0xFFCD,
    },
};

static esp_attr_value_t gatts_char_value; 

void ble_init(ble_driver_cfg_t *ble_cfg)
{
    memcpy(&ble_cfg_instance, ble_cfg, sizeof(ble_driver_cfg_t));

    adv_data.p_service_uuid = ble_cfg_instance.adv_service_uuid128;
    adv_data.service_uuid_len = sizeof(ble_cfg_instance.adv_service_uuid128);
    scan_rsp_data.p_service_uuid = ble_cfg_instance.adv_service_uuid128;
    scan_rsp_data.service_uuid_len = sizeof(ble_cfg_instance.adv_service_uuid128);

    gatts_char_value.attr_max_len = BLE_GATTS_CHAR_MAX_LEN;
    gatts_char_value.attr_len = sizeof(ble_cfg_instance.device_name);
    gatts_char_value.attr_value = (unsigned char*)ble_cfg_instance.device_name;

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
    ESP_ERROR_CHECK(esp_ble_gatts_app_register(ble_cfg->gatts_profile_id));
    ESP_ERROR_CHECK(esp_ble_gatt_set_local_mtu(512));

    esp_ble_auth_req_t auth_req = ESP_LE_AUTH_REQ_SC_MITM_BOND;
    esp_ble_io_cap_t io_cap = ESP_IO_CAP_NONE;
    uint8_t key_size = 16;
    uint8_t init_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    uint8_t rsp_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    uint32_t passkey = 123456;
    uint8_t auth_option = ESP_BLE_ONLY_ACCEPT_SPECIFIED_AUTH_DISABLE;
    uint8_t oob_support = ESP_BLE_OOB_DISABLE;

    ESP_ERROR_CHECK(esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(auth_req)));
    ESP_ERROR_CHECK(esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &io_cap, sizeof(io_cap)));
    ESP_ERROR_CHECK(esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, sizeof(key_size)));
    ESP_ERROR_CHECK(esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key, sizeof(init_key)));
    ESP_ERROR_CHECK(esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key, sizeof(rsp_key)));
    ESP_ERROR_CHECK(esp_ble_gap_set_security_param(ESP_BLE_SM_SET_STATIC_PASSKEY, &passkey, sizeof(passkey)));
    ESP_ERROR_CHECK(esp_ble_gap_set_security_param(ESP_BLE_SM_ONLY_ACCEPT_SPECIFIED_SEC_AUTH, &auth_option, sizeof(auth_option)));
    ESP_ERROR_CHECK(esp_ble_gap_set_security_param(ESP_BLE_SM_OOB_SUPPORT, &oob_support, sizeof(oob_support)));
}

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
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
        case ESP_GAP_BLE_AUTH_CMPL_EVT:
            ESP_LOGI(BLE_LOG_TAG, "Auth complete, remote: %02x:%02x:%02x:%02x:%02x:%02x, pair status: %s",
                     param->ble_security.auth_cmpl.bd_addr[0],
                     param->ble_security.auth_cmpl.bd_addr[1],
                     param->ble_security.auth_cmpl.bd_addr[2],
                     param->ble_security.auth_cmpl.bd_addr[3],
                     param->ble_security.auth_cmpl.bd_addr[4],
                     param->ble_security.auth_cmpl.bd_addr[5],
                     param->ble_security.auth_cmpl.success ? "success" : "fail");
            break;
        case ESP_GAP_BLE_SEC_REQ_EVT:
            esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
            break;
        default:
            break;
    }
}

static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
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
                ESP_LOGI(BLE_LOG_TAG, "App registration failed, app_id: %x, status %d", param->reg.app_id, param->reg.status);
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

static void gatts_profile_sensor_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch(event)
    {
        case ESP_GATTS_CONNECT_EVT:
        {
            esp_ble_conn_update_params_t conn_params = {0};
            memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
            conn_params.latency = 0;
            conn_params.max_int = 0x30;
            conn_params.min_int = 0x10;
            conn_params.timeout = 400;

            ESP_LOGI(BLE_LOG_TAG, "Connection event: conn_id: %d, remote: %02x:%02x:%02x:%02x:%02x:%02x",
                    param->connect.conn_id,  
                    param->connect.remote_bda[0],  
                    param->connect.remote_bda[1],  
                    param->connect.remote_bda[2],  
                    param->connect.remote_bda[3],  
                    param->connect.remote_bda[4],  
                    param->connect.remote_bda[5]);

            ESP_ERROR_CHECK(esp_ble_set_encryption(param->connect.remote_bda, ESP_BLE_SEC_ENCRYPT_MITM));
            ESP_ERROR_CHECK(esp_ble_gap_update_conn_params(&conn_params));
            break;
        }
        case ESP_GATTS_REG_EVT:
        {
            ESP_LOGI(BLE_LOG_TAG, "BLE Register App Event, status: %d, app_id: %d", param->reg.status, param->reg.app_id);
            ESP_ERROR_CHECK(esp_ble_gap_set_device_name(ble_cfg_instance.device_name));
            ESP_ERROR_CHECK(esp_ble_gap_config_adv_data(&adv_data));
            adv_config_done |= BLE_ADV_CONFIG_FLAG;
            ESP_ERROR_CHECK(esp_ble_gap_config_adv_data(&scan_rsp_data));
            adv_config_done |= BLE_ADV_SCAN_RSP_CONFIG_FLAG;
            ESP_ERROR_CHECK(esp_ble_gatts_create_service(gatts_if, &gatts_profile_instance.service_id, gatts_profile_instance.service_num_handles));
            break;
        }
        case ESP_GATTS_CREATE_EVT:
        {   
            ESP_LOGI(BLE_LOG_TAG, "Created service, status: %d, service_handle: %d", param->create.status, param->create.service_handle);
            gatts_profile_instance.service_handle = param->create.service_handle;
            ESP_ERROR_CHECK(esp_ble_gatts_start_service(gatts_profile_instance.service_handle));
            ESP_ERROR_CHECK(esp_ble_gatts_add_char(gatts_profile_instance.service_handle, 
                                                    &gatts_profile_instance.char_uuid, 
                                                    ESP_GATT_PERM_READ_ENCRYPTED, 
                                                    ESP_GATT_CHAR_PROP_BIT_READ, 
                                                    &gatts_char_value,
                                                    NULL));
            break;
        }
        case ESP_GATTS_ADD_CHAR_EVT: 
        {
            ESP_LOGI(BLE_LOG_TAG, "Created characteristic,: status: %d, attr_handle: %d, service_handle: %d",
                 param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);
            break;
        }
        case ESP_GATTS_READ_EVT:
        {
            ESP_LOGI(BLE_LOG_TAG, "Read event: conn_id: %d, trans_id: %ld, handle: %d",  
                     param->read.conn_id, param->read.trans_id, param->read.handle); 

            esp_gatt_rsp_t response;
            memset(&response, 0, sizeof(esp_gatt_rsp_t));

            ble_cfg_instance.response_cb(response.attr_value.value, &response.attr_value.len);
            ESP_LOGI(BLE_LOG_TAG, "Value from ADC: %s", (char*)response.attr_value.value);
            ESP_ERROR_CHECK(esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id, ESP_GATT_OK, &response));
            break;
        }
        case ESP_GATTS_DISCONNECT_EVT:
        {
            ESP_LOGI(BLE_LOG_TAG, "Disconnect event, reason: 0x%x", param->disconnect.reason);
            esp_ble_gap_start_advertising(&adv_params);
            break;
        }
        default:
            break;
    }
}