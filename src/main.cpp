//#include <lvgl.h>

#include <app/secrets.h>
#include <Arduino.h>
#include <Audio.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <device/JC4827W543.hpp>
#include <app/RadioStations.hpp>


//Core0 Task creation
TaskHandle_t Task1;

//Main device object
JC4827W543 dev;

//Radio stations repository and source file name declaration
RadioStations stations;
const char *stationFile = "/radio.json";

uint32_t screenActiveTime = 0;
const uint8_t screenOffTimeSeconds = 60;
uint32_t timeRefresh = 0;

//Interface must be included after all declarations
#include <app/Interface.hpp>


void refreshTimeLabel() {
  setTimeLabel(dev.timeGet("%H:%M \n %d/%m/%Y"));
}

void TaskForCore0(void * parameter) {
  for(;;) {
    lv_task_handler(); // let the GUI do its work
    ArduinoOTA.handle();
    if (millis() - screenActiveTime > screenOffTimeSeconds * 1000)
    {
      dev.graphic_backlight(false);
    }
    else
    {
      dev.graphic_backlight(true);
    }

    if (millis() - timeRefresh >= 30000) {
      refreshTimeLabel();
      timeRefresh = millis();
    }    
    vTaskDelay(pdMS_TO_TICKS(50)); // Stałe, bezpieczne opóźnienie
  }
} 

void setup()
{
  dev.init();
  stations.init(dev.SD_FileOpen(stationFile));
  
  dev.wifiInit(HOSTNAME, WIFIName, WIFIPasswd);
  dev.ntpInit(timeServer1, timeServer2, timeServer3);
  SetupLVGL();
  refreshTimeLabel();

  ArduinoOTA.setPassword(otaPassword);
  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.begin();

  xTaskCreatePinnedToCore(
      TaskForCore0, /* Function to implement the task */
      "Task1", /* Name of the task */
      10000,  /* Stack size in words */
      NULL,  /* Task input parameter */
      1,  /* Priority of the task */
      &Task1,  /* Task handle. */
      0); /* Core where the task should run */
      
    debugPrint("Setup done");

    screenActiveTime = millis();
}

void loop()
{
  dev.audio_loop();
  dev.wifiCheck();
  vTaskDelay(pdMS_TO_TICKS(1));
}



