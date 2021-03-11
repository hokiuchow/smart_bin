#include <stdio.h>
#include <string.h>
#include <driver/gpio.h>
#include <esp_event.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <freertos/event_groups.h>
#include <esp_camera.h>
#include <esp_http_client.h>

#include "connect_wifi.h"
#include "motor.h"
#include "motion.h"

#define CAMERA_TAG "CAMERA"
#define HTTP_TAG "HTTP CLIENT"
#define LED_PIN GPIO_NUM_4
#define FRAMESIZE FRAMESIZE_VGA // FRAMESIZE_UXGA

// #define URL "https://asia-east2-smart-bin-project-301204.cloudfunctions.net/image_classifer"  // GCP
#define URL "http://192.168.0.101:5000/api/test" // computer
// #define URL "http://192.168.0.123:5000/api/test" // lambda-quad
// #define URL "http://192.168.1.74:5000/api/test"

extern xSemaphoreHandle motion_sensor_semaphore;
char *buffer = NULL;
int bufferIndex = 0;

void turn_flash_on()
{
    gpio_set_level(LED_PIN, 1);
}

void turn_flash_off()
{
    gpio_set_level(LED_PIN, 0);
}

esp_err_t http_client_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        ESP_LOGI(HTTP_TAG, "HTTP_EVENT_ON_DATA Len=%d", evt->data_len);
        buffer = (buffer == NULL) ? (char *)malloc(evt->data_len) : (char *)realloc(buffer, evt->data_len + bufferIndex);
        memcpy(&buffer[bufferIndex], evt->data, evt->data_len);
        bufferIndex += evt->data_len;
        break;

    case HTTP_EVENT_ON_FINISH:
        ESP_LOGI(HTTP_TAG, "HTTP_EVENT_ON_FINISH");
        buffer = (char *)realloc(buffer, bufferIndex + 1);
        memcpy(&buffer[bufferIndex], "\0", 1);
        ESP_LOGI(HTTP_TAG, "Response: %s", buffer);

        servo_sweep(buffer);

        // Cleanup
        free(buffer);
        buffer = NULL;
        bufferIndex = 0;
        break;

    default:
        break;
    }

    return ESP_OK;
}

void send_photo_to_server(camera_fb_t *fb)
{
    static esp_http_client_config_t http_client_config = {
        .url = URL,
        .event_handler = http_client_event_handler,
        .method = HTTP_METHOD_POST};
    esp_http_client_handle_t client = esp_http_client_init(&http_client_config);
    esp_http_client_set_post_field(client, (const char *)fb->buf, fb->len);
    esp_http_client_set_header(client, "Content-Type", "image/jpg");
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK)
    {
        ESP_LOGI(HTTP_TAG, "HTTP POST request success. STATUS: %d", esp_http_client_get_status_code(client));
    }
    else
    {
        ESP_LOGE(HTTP_TAG, "HTTP POST request failed. STATUS: %d", esp_http_client_get_status_code(client));
    }
    esp_http_client_cleanup(client);
}

void camera_task(void *params)
{
    while (true)
    {
        xSemaphoreTake(motion_sensor_semaphore, portMAX_DELAY);
        wait_for_motion_to_stop();
        vTaskDelay(500 / portTICK_PERIOD_MS);

        // Send photo to server
        ESP_LOGI(CAMERA_TAG, "Taking photo...");
        turn_flash_on();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        camera_fb_t *fb = esp_camera_fb_get();
        turn_flash_off();

        ESP_LOGI(CAMERA_TAG, "Sending photo to server...");
        send_photo_to_server(fb);

        esp_camera_fb_return(fb);

        motion_sensor_start();
    }
}

void camera_start()
{
    // Init motion sensor
    motion_sensor_init();
    motion_sensor_start();

    // Init camera
    ESP_LOGI(CAMERA_TAG, "Initializing Camera");
    camera_config_t camera_config = {
        .pin_pwdn = CONFIG_PWDN,
        .pin_reset = CONFIG_RESET,
        .pin_xclk = CONFIG_XCLK,
        .pin_sscb_sda = CONFIG_SDA,
        .pin_sscb_scl = CONFIG_SCL,

        .pin_d7 = CONFIG_D7,
        .pin_d6 = CONFIG_D6,
        .pin_d5 = CONFIG_D5,
        .pin_d4 = CONFIG_D4,
        .pin_d3 = CONFIG_D3,
        .pin_d2 = CONFIG_D2,
        .pin_d1 = CONFIG_D1,
        .pin_d0 = CONFIG_D0,
        .pin_vsync = CONFIG_VSYNC,
        .pin_href = CONFIG_HREF,
        .pin_pclk = CONFIG_PCLK,

        //XCLK 20MHz or 10MHz
        .xclk_freq_hz = CONFIG_XCLK_FREQ,
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,

        .pixel_format = PIXFORMAT_JPEG, //YUV422,GRAYSCALE,RGB565,JPEG
        .frame_size = FRAMESIZE,        //QQVGA-UXGA Do not use sizes above QVGA when not JPEG

        .jpeg_quality = 12, //0-63 lower number means higher quality
        .fb_count = 2       //if more than one, i2s runs in continuous mode. Use only with JPEG
    };
    esp_camera_init(&camera_config);
    gpio_config_t led_gpio_config = {
        .pin_bit_mask = 1ULL << LED_PIN,
        .mode = GPIO_MODE_DEF_OUTPUT};
    gpio_config(&led_gpio_config);

    xTaskCreate(camera_task, "take photo", 8192, NULL, 2, NULL);
}