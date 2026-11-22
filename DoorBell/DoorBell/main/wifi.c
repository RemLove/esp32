#include "wifi.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "qrcode.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

#define PROV_QR_VERSION "v1"
#define QRCODE_BASE_URL "https://espressif.github.io/esp-jumpstart/qrcode.html"

static EventGroupHandle_t s_wifi_event_group;

static const char *TAG1 = "wifi station";

static int s_retry_num = 0;

static void get_device_service_name(char *service_name, size_t max)
{
    uint8_t eth_mac[6];
    const char *ssid_prefix = "PROV_";
    esp_wifi_get_mac(WIFI_IF_STA, eth_mac);
    snprintf(service_name, max, "%s%02X%02X%02X",
             ssid_prefix, eth_mac[3], eth_mac[4], eth_mac[5]);
}

static void wifi_prov_print_qr(const char *name, const char *username, const char *pop, const char *transport)
{
    if (!name || !transport)
    {
        ESP_LOGW(TAG1, "Cannot generate QR code payload. Data missing.");
        return;
    }
    char payload[150] = {0};
    if (pop)
    {
        snprintf(payload, sizeof(payload), "{\"ver\":\"%s\",\"name\":\"%s\""
                                           ",\"pop\":\"%s\",\"transport\":\"%s\"}",
                 PROV_QR_VERSION, name, pop, transport);
    }
    else
    {
        snprintf(payload, sizeof(payload), "{\"ver\":\"%s\",\"name\":\"%s\""
                                           ",\"transport\":\"%s\",\"network\":\"wifi\"}",
                 PROV_QR_VERSION, name, transport);
    }
    ESP_LOGI(TAG1, "Scan this QR code from the provisioning application for Provisioning.");
    esp_qrcode_config_t cfg = ESP_QRCODE_CONFIG_DEFAULT();
    esp_qrcode_generate(&cfg, payload);

    ESP_LOGI(TAG1, "If QR code is not visible, copy paste the below URL in a browser.\n%s?data=%s", QRCODE_BASE_URL, payload);
}

/* 启动 STA：配过网时用。凭证存在 NVS 里（esp_wifi_init 时自动恢复），
   esp_wifi_start() 触发 WIFI_EVENT_STA_START，事件回调里 esp_wifi_connect() 完成连接 */
static void wifi_init_sta(void)
{
    /* Start Wi-Fi in station mode */
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < 5)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG1, "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG1, "connect to the AP fail");
    }
    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {

        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG1, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
    if (event_base == NETWORK_PROV_EVENT)
    {
        switch (event_id)
        {
        case NETWORK_PROV_START:
            ESP_LOGI(TAG1, "Provisioning started");
            break;
        case NETWORK_PROV_WIFI_CRED_RECV:
        {
            wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
            ESP_LOGI(TAG1, "Received Wi-Fi credentials"
                           "\n\tSSID     : %.*s\n\tPassword : %.*s",
                     (int)sizeof(wifi_sta_cfg->ssid),
                     (const char *)wifi_sta_cfg->ssid,
                     (int)sizeof(wifi_sta_cfg->password),
                     (const char *)wifi_sta_cfg->password);
            break;
        }
        case NETWORK_PROV_WIFI_CRED_FAIL:
        {
            network_prov_wifi_sta_fail_reason_t *reason = (network_prov_wifi_sta_fail_reason_t *)event_data;
            ESP_LOGE(TAG1, "Provisioning failed!\n\tReason : %s"
                           "\n\tPlease reset to factory and retry provisioning",
                     (*reason == NETWORK_PROV_WIFI_STA_AUTH_ERROR) ? "Wi-Fi station authentication failed" : "Wi-Fi access-point not found");
            // 如果失败 复位
            network_prov_mgr_reset_wifi_sm_state_on_failure();
            break;
        }
        case NETWORK_PROV_WIFI_CRED_SUCCESS:
            ESP_LOGI(TAG1, "Provisioning successful");
            break;
        case NETWORK_PROV_END:
        {
            /* De-initialize manager once provisioning is finished */
            esp_err_t err = network_prov_mgr_deinit();
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG1, "Failed to de-initialize provisioning manager: %s", esp_err_to_name(err));
            }
            break;
        }
        default:
            break;
        }
    }
    if (event_base == PROTOCOMM_SECURITY_SESSION_EVENT)
    {
        switch (event_id)
        {
        case PROTOCOMM_SECURITY_SESSION_SETUP_OK:
            ESP_LOGI(TAG1, "Secured session established!");
            break;
        case PROTOCOMM_SECURITY_SESSION_INVALID_SECURITY_PARAMS:
            ESP_LOGE(TAG1, "Received invalid security parameters for establishing secure session!");
            break;
        case PROTOCOMM_SECURITY_SESSION_CREDENTIALS_MISMATCH:
            ESP_LOGE(TAG1, "Received incorrect username and/or PoP for establishing secure session!");
            break;
        default:
            break;
        }
    }
}

void Wifi_NVS_Init(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

void Wifi_Init(void)
{
    /* NVS 先行：配网凭证就存在这里 */
    Wifi_NVS_Init();

    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /* 事件回调只注册这一次，配网/连接两条路径共用 */
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    esp_event_handler_instance_t instance_prov;
    esp_event_handler_instance_t instance_security;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(NETWORK_PROV_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_prov));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(PROTOCOMM_SECURITY_SESSION_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_security));

    network_prov_mgr_config_t config = {
        .scheme = network_prov_scheme_ble,
        /* 注意：这个"handler"不是枚举值，是展开成 { .event_cb = ..., .user_data = NULL }
           的结构体宏，不能传 5 或 (void*)5。
           FREE_BTDM = 配网结束后释放蓝牙+双模控制器内存，
           门铃配完网就不再需要蓝牙了 */
        .scheme_event_handler = NETWORK_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM,
    };
    // 初始化配网信息
    ESP_ERROR_CHECK(network_prov_mgr_init(config));
    // 检查是否配过网
    bool provisioned = false;
    ESP_ERROR_CHECK(network_prov_mgr_is_wifi_provisioned(&provisioned));
    if (!provisioned) // 如果没配过网
    {
        ESP_LOGI(TAG1, "Starting provisioning");

        /* What is the Device Service Name that we want
         * This translates to :
         *     - Wi-Fi SSID when scheme is network_prov_scheme_softap
         *     - device name when scheme is network_prov_scheme_ble
         */
        char service_name[12];
        get_device_service_name(service_name, sizeof(service_name));

        network_prov_security_t security = NETWORK_PROV_SECURITY_1;
        const char *pop = "abcd1234";
        /* This is the structure for passing security parameters
         * for the protocomm security 1.
         */
        network_prov_security1_params_t *sec_params = pop;
        const char *username = NULL;
        // 蓝牙uuid
        uint8_t custom_service_uuid[] = {
            /* LSB <---------------------------------------
             * ---------------------------------------> MSB */
            0xb4,
            0xdf,
            0x5a,
            0x1c,
            0x3f,
            0x6b,
            0xf4,
            0xbf,
            0xea,
            0x4a,
            0x82,
            0x03,
            0x04,
            0x90,
            0x1a,
            0x02,
        };
        network_prov_scheme_ble_set_service_uuid(custom_service_uuid);
        ESP_ERROR_CHECK(network_prov_mgr_start_provisioning(security, (const void *)sec_params, service_name, NULL));
        // 打印二维码（手机推凭证后 manager 自动写 NVS 并连接，无需手动 set_config）
        wifi_prov_print_qr(service_name, username, pop, "ble");
    }
    else // 如果配过网
    {
        ESP_LOGI(TAG1, "Already provisioned, starting Wi-Fi STA");

        /* We don't need the manager as device is already provisioned,
         * so let's release it's resources */
        ESP_ERROR_CHECK(network_prov_mgr_deinit());

        /* Start Wi-Fi station（凭证用 NVS 里手机配网写入的那份） */
        wifi_init_sta();
    }

    /* 统一等待：连接成功 or 五次重试全失败 */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG1, "connected to AP");
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGE(TAG1, "Failed to connect to AP after retries, consider re-provisioning");
    }
    else
    {
        ESP_LOGE(TAG1, "UNEXPECTED EVENT");
    }
}
