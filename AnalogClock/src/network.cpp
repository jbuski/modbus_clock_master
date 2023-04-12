#include <Wifi.h>
#include <ArduinoOTA.h>
#include <NTPClient_Generic.h>
#include "AnalogClock.h"
#include "cmdArduino.h"
#include "network.h"

#define TIME_ZONE_OFFSET_HRS 1 // UTC+1 for Germany, winter time
#define NTP_UPDATE_INTERVAL_MS 60000L // Update every 60s automatically

#define DEBUG_LEVEL  1                                               // manages most of the print and println debug, not all but most

#if (DEBUG_LEVEL > 0U)

#define UsrLogln(...)\
		Serial.print("USR network.cpp: ");\
		Serial.println(__VA_ARGS__)
		
#define UsrLog(...)\
		Serial.print("USR network.cpp: ");\
		Serial.print(__VA_ARGS__)
#define print_k(...)\
		Serial.print(__VA_ARGS__)
		
#define print_kln(...)\
		Serial.println(__VA_ARGS__)
#else
#define UsrLogln(...)
#define UsrLog(...)
#define print_k(...)
#define print_kln(...)
#endif

#if (DEBUG_LEVEL > 1U)
#define ErrLogln(...)\
		Serial.print("ERR network.cpp: ");\
		Serial.println(__VA_ARGS__)
		
#define ErrLog(...)\
		Serial.print("ERR network.cpp: ");\
		Serial.print(__VA_ARGS__)
#else
#define ErrLogln(...)
#define ErrLog(...)
#endif

#if (DEBUG_LEVEL > 2U)
#define DbgLogln(...)\
		Serial.print("DBG network.cpp: ");\
		Serial.println(__VA_ARGS__)
		
#define DbgLog(...)\
		Serial.print("DBG network.cpp: ");\
		Serial.print(__VA_ARGS__)
#else
#define DbgLogln(...)
#define DbgLog(...)
#endif

extern wifiConnected_t wifiConnected;

extern const char* ssid;
extern const char* password;
extern uint8_t start_flag;

extern AnalogClock my_clock;

TaskHandle_t ntp_task1_handle = NULL;

static bool wifiFirstConnected;

unsigned long last = 0;

WiFiUDP ntpUDP;
NTPClient ntpClient(ntpUDP, "pool.ntp.org", (3600 * TIME_ZONE_OFFSET_HRS), NTP_UPDATE_INTERVAL_MS);

void initWiFi(const char *host_name) 
{
  uint8_t i = 0;
  WiFi.disconnect(true);
  WiFi.setHostname(host_name);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.onEvent(WiFiEvent);
  Serial.print("[M] Connecting to WiFi ");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    i++;
    my_clock.SetProgress(i);
    delay(100);
    if (i>=100)
    {
      Serial.println("Failed");
      Serial.println("Rebooting ..");
      ESP.restart();
    }
  }
  Serial.println("Connected");
}

void get_network_info(void){
    if(WiFi.status() == WL_CONNECTED) {
        Serial.print("[*] Network information for ");
        Serial.println(ssid);
        Serial.println("[+] BSSID : " + WiFi.BSSIDstr());
        Serial.print("[+] Gateway IP : ");
        Serial.println(WiFi.gatewayIP());
        Serial.print("[+] Subnet Mask : ");
        Serial.println(WiFi.subnetMask());
        Serial.println((String)"[+] RSSI : " + WiFi.RSSI() + " dB");
        Serial.print("[+] ESP32 IP : ");
        Serial.println(WiFi.localIP());
        Serial.printf("[+] ESP32 Chip model %s Rev %d\n\r", ESP.getChipModel(),
                  ESP.getChipRevision());
        Serial.printf("[+] This chip has %d cores\n\r", ESP.getChipCores());
    }
}

void start_ota(void)
{ 
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  // ArduinoOTA.setHostname("myesp32");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");
  
}

void ntpUpdateTask(void* param) 
{
  while(true) {
    // Update NTP. This will only ACTUALLY update if
    // the NTP client has not updated for NTP_UPDATE_INTERVAL_MS
    ntpClient.update();
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    //vTaskDelete(nullptr);
  }
}

void ntpStartStopTask(void* param)
{
    while(true) {
        switch (wifiConnected)
        {
            case CONNECTED:
            start_ntp();
            wifiConnected = IDLE;
            break;

            case DISCONNECTED:
            stop_ntp();
            wifiConnected = IDLE;
            break;

            case IDLE:
            default:
            break;
        }
        vTaskDelay(1000);
    }
}

void init_ntp(void)
{
    ntpClient.begin();

    print_k("[N] Starting NTP ntpStartStopTask ... ");
    //print_k("NTP begin ... ");
    //ntpClient.begin();
    print_k("creating task ... ");
    xTaskCreate(ntpStartStopTask, "NTP update", 2000, nullptr, 1, nullptr);
    print_kln("Done");
}

void start_ntp(void)
{
    delay(5000);
    print_k("[N] Starting NTP ntpUpdateTask ... ");
    //print_k("NTP begin ... ");
    //ntpClient.begin();
    print_k("creating task ... ");
    xTaskCreate(ntpUpdateTask, "NTP update", 2000, nullptr, 1, &ntp_task1_handle);
    print_kln("Done");
}

void stop_ntp(void)
{
    print_k("[N] Stopping NTP ... ");
    print_k("suspeding task ... ");
    vTaskSuspend(ntp_task1_handle);
    print_k("deleting task ... ");
    vTaskDelete(ntp_task1_handle);
    print_kln("Done");
}

void get_task_list()
{
    print_kln("[N] RTOS task list");
    //vTaskList(ptrTaskList);
    Serial.println("**********************************");
    Serial.println("Task  State   Prio    Stack    Num"); 
    Serial.println("**********************************");
    //Serial.print(ptrTaskList);
    Serial.println("**********************************");
    print_kln("Done");
}

void connect_ntp(void)
{
   // if (wifiFirstConnected) 
    //{
    //    wifiFirstConnected = false;
    //    start_ntp();

    //}

    if ((millis () - last) > 2000) 
    {
        if (ntpClient.updated()) 
        {
            //Serial.println("# Time in sync with NTP server");
            //start_flag = 1;
        } 
        else 
        {
            //Serial.println("# TIME NOT IN SYNC WITH NTP SERVER !");
            //start_flag = 0;
        }
        last = millis ();
    }
}

uint8_t get_hour(void)\
{
    return (uint8_t) ntpClient.getUTCHours();
}

uint8_t get_minute(void)
{
    return (uint8_t) ntpClient.getUTCMinutes();
}

uint8_t get_second(void)
{
    return (uint8_t) ntpClient.getUTCSeconds();
}

void WiFiEvent(WiFiEvent_t event)
{
    //Serial.printf("[WiFi-event] event: %d\n", event);

    switch (event) {
        case ARDUINO_EVENT_WIFI_READY: 
            Serial.println("[E] WiFi interface ready");
            break;
        case ARDUINO_EVENT_WIFI_SCAN_DONE:
            Serial.println("[E] Completed scan for access points");
            break;
        case ARDUINO_EVENT_WIFI_STA_START:
            Serial.println("[E] WiFi client started");
            break;
        case ARDUINO_EVENT_WIFI_STA_STOP:
            Serial.println("[E] WiFi clients stopped");
            break;
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            //Serial.println("[E] Connected to access point");
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Serial.println("[E] Disconnected from WiFi access point");
            //stop_ntp();
            wifiConnected = DISCONNECTED;
            break;
        case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
            Serial.println("[E] Authentication mode of access point has changed");
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            //Serial.print("[E] Obtained IP address: ");
            Serial.println(WiFi.localIP());
            //start_ntp();
            wifiConnected = CONNECTED;
            break;
        case ARDUINO_EVENT_WIFI_STA_LOST_IP:
            Serial.println("[E] Lost IP address and IP address is reset to 0");
            break;
        case ARDUINO_EVENT_WPS_ER_SUCCESS:
            Serial.println("[E] WiFi Protected Setup (WPS): succeeded in enrollee mode");
            break;
        case ARDUINO_EVENT_WPS_ER_FAILED:
            Serial.println("[E] WiFi Protected Setup (WPS): failed in enrollee mode");
            break;
        case ARDUINO_EVENT_WPS_ER_TIMEOUT:
            Serial.println("[E] WiFi Protected Setup (WPS): timeout in enrollee mode");
            break;
        case ARDUINO_EVENT_WPS_ER_PIN:
            Serial.println("[E] WiFi Protected Setup (WPS): pin code in enrollee mode");
            break;
        case ARDUINO_EVENT_WIFI_AP_START:
            Serial.println("[E] WiFi access point started");
            break;
        case ARDUINO_EVENT_WIFI_AP_STOP:
            Serial.println("[E] WiFi access point stopped");
            break;
        case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
            Serial.println("[E] Client connected");
            break;
        case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
            Serial.println("[E]Client disconnected");
            break;
        case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
            Serial.println("[E] Assigned IP address to client");
            break;
        case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
            Serial.println("[E] Received probe request");
            break;
        case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
            Serial.println("[E] AP IPv6 is preferred");
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
            Serial.println("[E] STA IPv6 is preferred");
            break;
        case ARDUINO_EVENT_ETH_GOT_IP6:
            Serial.println("[E] Ethernet IPv6 is preferred");
            break;
        case ARDUINO_EVENT_ETH_START:
            Serial.println("[E] Ethernet started");
            break;
        case ARDUINO_EVENT_ETH_STOP:
            Serial.println("[E] Ethernet stopped");
            break;
        case ARDUINO_EVENT_ETH_CONNECTED:
            Serial.println("[E] Ethernet connected");
            break;
        case ARDUINO_EVENT_ETH_DISCONNECTED:
            Serial.println("[E] Ethernet disconnected");
            break;
        case ARDUINO_EVENT_ETH_GOT_IP:
            Serial.println("[E] Obtained IP address");
            break;
        default: break;
    }
}