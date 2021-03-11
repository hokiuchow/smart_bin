#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <freertos/event_groups.h>

#include "connect_wifi.h"
#include "ota_listener.h"
#include "camera.h"
#include "motor.h"

void app_main(void)
{
  wifi_start();
  xEventGroupWaitBits(wifiEventGroup, WIFI_CONNECTED, false, true, portMAX_DELAY);

  ota_listener_start();
  camera_start();
  servo_test();
}