#ifndef _CONNECT_WIFI_H
#define _CONNECT_WIFI_H
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

EventGroupHandle_t wifiEventGroup;
#define WIFI_CONNECTED BIT0

void wifi_start();

#endif