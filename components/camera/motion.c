#include <stdio.h>
#include <string.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include "motion.h"

#define SENSOR_PIN GPIO_NUM_13
#define ENABLE_PIN GPIO_NUM_15

#define TAG "MOTION"

xSemaphoreHandle motion_sensor_semaphore;

void on_motion()
{
    gpio_isr_handler_remove(SENSOR_PIN);
    xSemaphoreGiveFromISR(motion_sensor_semaphore, pdFALSE);
}

void motion_sensor_init()
{
    ESP_LOGI(TAG, "Initializing motion sensor...");
    gpio_config_t motion_sensor_gpio_config = {
        .pin_bit_mask = 1ULL << SENSOR_PIN,
        .mode = GPIO_MODE_DEF_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLUP_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE};
    gpio_config(&motion_sensor_gpio_config);

    gpio_set_direction(ENABLE_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(ENABLE_PIN, 1);

    gpio_install_isr_service(0);

    motion_sensor_semaphore = xSemaphoreCreateBinary();
}

void motion_sensor_start()
{
    ESP_LOGI(TAG, "Starting motion sensor...");
    gpio_isr_handler_add(SENSOR_PIN, on_motion, NULL);
    ESP_LOGI(TAG, "Waiting for motion...");
}

void wait_for_motion_to_stop()
{
    do
    {
        ESP_LOGI(TAG, "Stopping...");
        vTaskDelay(20 / portTICK_PERIOD_MS);
    } while (gpio_get_level(SENSOR_PIN) == 0);
}