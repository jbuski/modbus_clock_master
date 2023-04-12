#ifndef network_h
#define network_h

#include "Arduino.h"
#include <WiFi.h>

typedef enum wifiConnected 
    {
        IDLE, 
        CONNECTED, 
        DISCONNECTED
    }wifiConnected_t;

void initWiFi(const char * host_name);

void get_network_info(void);

void ntpUpdateTask(void* param);

void start_ntp(void);

void init_ntp(void);

void stop_ntp(void);

void start_ota(void);

void WiFiEvent(WiFiEvent_t event);

void connect_ntp(void);

void get_task_list(void);

uint8_t get_hour(void);

uint8_t get_minute(void);

uint8_t get_second(void);

#endif
