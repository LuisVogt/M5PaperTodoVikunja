#include <M5EPD.h>
#include <List>
#include "FS.h"
#include "SPIFFS.h"
#include "binaryttf.h"
#include "ToDoAPI/ToDoAPI.h"
#include "GUI/ButtonGUI.h"
#include "GUI/ListGUI.h"
#include "GlobalConfig.h"
#include <sstream>

//Example for TTF font
#define USE_SPIFFS

M5EPD_Canvas mainCanvas(&M5.EPD);
ListGUI listGUI;
const int WAKEUP_PIN = 37;

esp_err_t connectWiFi()
{
    String ssid="Luis";
    String password="Frajola123";
    if((ssid.length() < 2) || (password.length() < 8))
    {
        return ESP_FAIL;
    }
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    log_d("Connect to %s", ssid.c_str());
    uint32_t t = millis();
    while (1)
    {
        if(millis() - t > 8000)
        {
            WiFi.disconnect();
            return ESP_FAIL;
        }

        if(WiFi.status() == WL_CONNECTED)
        {
            return ESP_OK;
        }
    }
}

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

void setup()
{
    M5.begin();        
    //M5.disableEXTPower();
    M5.TP.SetRotation(90);
    M5.EPD.SetRotation(90);
    M5.RTC.begin();
    //M5.shutdown(-1);
    if (!SPIFFS.begin(true))
    {
        log_e("SPIFFS Mount Failed");
        while(1);
    }
    if (!SD.begin()) {
        log_e("SD Mount Failed");
        while(1);
    }
    configuration.LoadConfiguration();
    esp_err_t wifi_err = ESP_FAIL;
    wifi_err = connectWiFi();
    std::list<Task> Tasks;
 
    if(wifi_err == ESP_OK)
    {    
        listGUI.Setup();
        listGUI.CreateButtons();
    }   



    mainCanvas.loadFont("/GenSenRounded-R.ttf", SD); // Load font files from SPIFFS
    M5.EPD.Clear(true);
    mainCanvas.createCanvas(configuration.getScreenWidth(),configuration.getScreenHeight());
    
    listGUI.Draw(mainCanvas);
    mainCanvas.pushCanvas(0, 0, UPDATE_MODE_GC16);

    //  attachInterrupt(WAKEUP_PIN, print_wakeup_reason, RISING);
    // // esp_err_t error1 = esp_sleep_enable_ext1_wakeup(1ULL << WAKEUP_PIN, ESP_EXT1_WAKEUP_ALL_LOW); // wake up on low-to-high transition
    //  esp_err_t error2 = esp_sleep_enable_timer_wakeup(1000ULL * 1000 * configuration.getSleepTime()); // sleep until next update
    // // esp_err_t error3 = esp_sleep_enable_ext0_wakeup(GPIO_NUM_37, 1);
    // gpio_hold_en(GPIO_NUM_37);
    // gpio_deep_sleep_hold_en();
    // esp_sleep_enable_ext0_wakeup(GPIO_NUM_37, 1);
    
    //log_d("Sleep set for %d", configuration.getSleepTime());

    
    //M5.shutdown(sleepTime);
    //M5.Power.timerSleep(600);
    //delay(1000);
    //esp_deep_sleep_start();
}

void loop()
{   
    M5.update();
    listGUI.Update(mainCanvas);
    if(millis() > configuration.getTimeToSleep())
    {
        toDoAPI.Begin(DEFAULT_TIME_ZONE);
        rtc_time_t CurrentTime;
        M5.RTC.getTime(&CurrentTime);
        
        rtc_time_t NextTime;
        int interval = 24/configuration.getNumberOfTimesToWakePerDay();
        NextTime.hour = (1+CurrentTime.hour/interval)*interval;
        NextTime.hour = NextTime.hour>=24? 0: NextTime.hour;
        //NextTime.hour = CurrentTime.hour;
        NextTime.min = 00;
        NextTime.sec = 00;
        log_d("Shutting down until %d",((NextTime.hour-CurrentTime.hour)*60*60+(NextTime.min-CurrentTime.min)*60+(NextTime.sec-CurrentTime.sec)));
        //esp_deep_sleep_start();
        delay(1000);
        //M5.shutdown((CurrentTime.hour-NextTime.hour)*60*60+(CurrentTime.min-NextTime.min)*60+(CurrentTime.sec-NextTime.sec));
        M5.shutdown(NextTime);
        // attachInterrupt(WAKEUP_PIN, print_wakeup_reason, RISING);
        // esp_deep_sleep_start();
        log_d("shat");
        //delay(100000);
    }
    delay(15);
}