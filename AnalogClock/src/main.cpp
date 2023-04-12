
#include "AnalogClock.h"
#include "cmdArduino.h"
#include <WiFi.h> //ESP8266 Core WiFi Library (you most likely already have this in your sketch)

#include <PubSubClient.h>
#include <ESP32Ping.h>

#include <DNSServer.h> //Local DNS Server used for redirecting all requests to the configuration portal
// #include <WiFiManager.h>
#include <WiFiUdp.h>

#include <ArduinoOTA.h>
// #include <Adafruit_NeoPixel.h>
#include "menu.h"
#include "network.h"
#include "FreeRTOSConfig.h"

#include "modbus.h"

#define CONFIG_FREERTOS_USE_TRACE_FACILITY 1
#define CONFIG_FREERTOS_USE_STATS_FORMATTING_FUNCTIONS 1

// Macro to read build flags
#define ST(A) #A
#define STR(A) ST(A)

#ifdef WIFI_SSID
const char *ssid = STR(WIFI_SSID);
#endif

#ifdef WIFI_PASSWORD
const char *password = STR(WIFI_PASSWORD);
#endif

#ifdef MQTT_SERVER
const char *mqtt_server = STR(MQTT_SERVER);
#endif

#ifdef MQTT_PORT
const int mqtt_port = atoi(STR(MQTT_PORT));
#endif

#ifdef MQTT_USERNAME
const char *mqtt_username = STR(MQTT_USERNAME);
#endif

#ifdef MQTT_PASSWORD
const char *mqtt_password = STR(MQTT_PASSWORD);
#endif

#ifdef DEVICE_NAME
const char *device_name = STR(DEVICE_NAME);
#endif

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

int8_t timeZone = 2;
int8_t minutesTimeZone = 0;

wifiConnected_t wifiConnected;

unsigned long start_flag_time = 0;

const char *topic_command = "esp32/command";

char ptrTaskList[250];

#define hours_pin 25
#define minutes_pin 27
#define seconds_pin 14
#define backlight 4
#define AM_PM 2

#define OTA 1

AnalogClock my_clock;

uint8_t start_flag = 1;

uint8_t h, s, m;

String clientId = "esp32-client-";

void callback(char *topic, byte *message, unsigned int length);

WiFiClient espClient;
PubSubClient client(mqtt_server, mqtt_port, callback, espClient);

void callback(char *topic, byte *message, unsigned int length)
{
    Serial.print("Message arrived on topic: ");
    Serial.print(topic);
    Serial.print(". Message: ");
    String messageTemp;

    for (int i = 0; i < length; i++)
    {
        Serial.print((char)message[i]);
        messageTemp += (char)message[i];
    }
    Serial.println();

    // Feel free to add more if statements to control more GPIOs with MQTT

    // If a message is received on the topic esp32/output, you check if the message is either "on" or "off".
    // Changes the output state according to the message
    if (String(topic) == "esp32/output")
    {
        Serial.print("Changing output to ");
        if (messageTemp == "on")
        {
            Serial.println("on");
            // digitalWrite(ledPin, HIGH);
        }
        else if (messageTemp == "off")
        {
            Serial.println("off");
            // digitalWrite(ledPin, LOW);
        }
    }
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.printf("[M] Attempting MQTT connection to %s\n\r", mqtt_server);

    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password))
    {
      Serial.printf("[M] Connected as clientId %s :-)\n\r", clientId.c_str());

      // Subscribe
      client.subscribe(topic_command, 1);
      Serial.printf("[i] Subscribe to the topic command %s \n\r", topic_command);
    }
    else
    {
      Serial.printf("[F] {failed, rc=%d try again in 5 seconds}\n\r" , client.state());
      delay(5000);
    }
  }
}

void setup()
{
    Serial.begin(115200);

    while (!Serial);

    cmd.begin(115200);
    Serial.println("[M] Booting");

    modbus_init();

    my_clock.Init(hours_pin, minutes_pin, seconds_pin, backlight, AM_PM);

    Serial.printf("[M] Connecting to WiFi network: %s\n\r", ssid);

    initWiFi(device_name);

    my_clock.test();

    start_ota();

    get_network_info();

    menu_init();

    init_ntp();

    Serial.println();
    start_com_poll();

    bool success = Ping.ping(mqtt_server, 3);

    if (!success)
    {
        Serial.printf("[F] Ping failed to %s\n\r", mqtt_server);
    }
    else
    {
        Serial.printf("[i] Ping OK to %s\n\r", mqtt_server);
    }
    Serial.printf("[M] Connecting to mqtt network: %s:%d\n\r",
                  mqtt_server, mqtt_port);
    Serial.println("[M] Ready");
}

void loop()
{
    ArduinoOTA.handle();

    connect_ntp();

    if (!client.connected())
    {
        reconnect();
    }

    // cmd.poll();

    if (start_flag)
    {
        // vTaskList(ptrTaskList);
        // Serial.print(ptrTaskList);
        s = get_second();
        m = get_minute();
        h = get_hour();
    }
    else
    {
        if ((millis() - start_flag_time) > 60000)
        {
            start_flag = 1;
        }
    }
    my_clock.Set_time(h, m, s);
    my_clock.Send_cal_values();
    my_clock.Convert();
    my_clock.Display();
    my_clock.DisplayAM_PM();
}
