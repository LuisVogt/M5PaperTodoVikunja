#pragma once
#include <ArduinoJson.h>

class Configuration
{
    public:
        Configuration();
        ~Configuration();
        void LoadConfiguration();
        int getScreenHeight();
        int getScreenWidth();
        int getHeaderHeight();
        int getNumberOfHeaderElements();
        int getHeaderWidth();
        int getNumberOfLines();
        int getLineHeight();
        int getSleepTime();
        int getNumberOfTimesToWakePerDay();
        int getBatteryMax();
        int getBatteryMin();
        String getSsid();
        String getWifiPassword();
        String getUrl();
        int getTimezone();
        String getToDoUser();
        String getToDoPassword();
        String getMainProjectName();
        String getSecondaryProjectName();
        String getTertiaryProjectName();
        bool getShowDone();
        uint32_t getTimeToSleep();

    private:
        DynamicJsonDocument *_json_doc;

        String url;

        int screenHeight;
        int screenWidth;

        int headerHeight;
        int numberOfHeaderElements;
        int numberOfLines;

        int sleepTime; //12 hours
        int numberOfTimesToWakePerDay;
        int batteryMax;
        int batteryMin;

        bool showDone;

        String ssid;
        String wifiPassword;

        int timeZone;
        
        String toDoUser;
        String toDoPassword;
        String mainProjectName;
        String secondaryProjectName;
        String tertiaryProjectName;
        uint32_t timeToSleep;
};

inline Configuration configuration;