// On computer:
// mosquitto -v -c ./mosquitto-demo.conf
// mosquitto_pub -d -h localhost -m hello -t /ota_update/1

#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include "freertos/task.h"
#include <freertos/semphr.h>
#include "freertos/event_groups.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "mqtt_client.h"

#include "ota.h"

#define TAG "MQTT"
#define MQTT_URI "mqtt://192.168.0.101:1883" //

TaskHandle_t taskHandle;
extern xSemaphoreHandle ota_semaphore;
#define MQTT_CONNECTED BIT1
#define OTA_TRIGGERED BIT2

void mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        xTaskNotify(taskHandle, MQTT_CONNECTED, eSetValueWithOverwrite);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        xTaskNotify(taskHandle, OTA_TRIGGERED, eSetValueWithOverwrite);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    mqtt_event_handler_cb(event_data);
}

void Connect_MQTT(void *para)
{
    ESP_LOGI(TAG, "Connecting MQTT...");
    esp_mqtt_client_config_t mqtt_config = {
        .uri = MQTT_URI,
        // .username = "station",
        // .password = "bledemo"
    };
    esp_mqtt_client_handle_t mqtt_client = NULL;
    mqtt_client = esp_mqtt_client_init(&mqtt_config);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, mqtt_client);
    esp_mqtt_client_start(mqtt_client);

    uint32_t command = 0;

    while (true)
    {
        xTaskNotifyWait(0, 0, &command, portMAX_DELAY);
        switch (command)
        {
        case MQTT_CONNECTED:
            esp_mqtt_client_subscribe(mqtt_client, "/ota_update/1", 0);
            ESP_LOGI(TAG, "Subscribed to /ota_update/1...");
            break;
        case OTA_TRIGGERED:
            ESP_LOGI(TAG, "STARTING OTA...");
            xSemaphoreGive(ota_semaphore);
            break;
        default:
            break;
        }
    }
}

void ota_listener_start()
{
    xTaskCreate(&Connect_MQTT, "handle comms", 1024 * 5, NULL, 5, &taskHandle);
}