#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <driver/mcpwm.h>
#include <soc/mcpwm_periph.h>
#include <esp_log.h>

#define SERVO_TAG "SERVO"
#define SERVO_PIN GPIO_NUM_12 //Set GPIO 12 as PWM0A, to which servo is connected

#define SERVO_MIN_PULSEWIDTH 1000 //Minimum pulse width in microsecond
#define SERVO_MAX_PULSEWIDTH 2000 //Maximum pulse width in microsecond
#define SERVO_MAX_DEGREE 180      //Maximum angle in degree upto which servo can rotate

static mcpwm_config_t pwm_config = {
    .frequency = 50, //frequency = 50Hz, i.e. for every servo motor time period should be 20ms
    .cmpr_a = 0,     //duty cycle of PWMxA = 0
    .cmpr_b = 0,     //duty cycle of PWMxb = 0
    .counter_mode = MCPWM_UP_COUNTER,
    .duty_mode = MCPWM_DUTY_MODE_0};

static uint32_t servo_per_degree_init(uint32_t degree_of_rotation)
{
    uint32_t cal_pulsewidth = 0;
    cal_pulsewidth = (SERVO_MIN_PULSEWIDTH + (((SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * (degree_of_rotation)) / (SERVO_MAX_DEGREE)));
    return cal_pulsewidth;
}

void servo_sweep(void *params)
{
    char res[] = "True";

    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, SERVO_PIN);
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config); //Configure PWM0A & PWM0B with above settings

    if (strcmp(res, (char *)params) == 0)
    {
        ESP_LOGI(SERVO_TAG, "Angle of rotation: 10");
        mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, servo_per_degree_init(20));
        vTaskDelay(300); // It takes 100ms/60degree for servo to rotate at 5V

        ESP_LOGI(SERVO_TAG, "Angle of rotation: 90");
        mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, servo_per_degree_init(115));
        vTaskDelay(100);
    }
    else
    {
        ESP_LOGI(SERVO_TAG, "Angle of rotation: 170");
        mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, servo_per_degree_init(220));
        vTaskDelay(300);

        ESP_LOGI(SERVO_TAG, "Angle of rotation: 90");
        mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, servo_per_degree_init(115));
        vTaskDelay(100);
    }
}

void servo_test()
{
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, SERVO_PIN);
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config); //Configure PWM0A & PWM0B with above settings

    mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, servo_per_degree_init(30));
    vTaskDelay(300);
    mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, servo_per_degree_init(210));
    vTaskDelay(300);
    mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, servo_per_degree_init(115));
    vTaskDelay(300);
    // for (int ang = 0; ang < 271; ang = ang + 10)
    // {
    //     mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, servo_per_degree_init(ang));
    //     ESP_LOGI(SERVO_TAG, "Angle of rotation: %i", ang);
    //     vTaskDelay(50);
    // }
}