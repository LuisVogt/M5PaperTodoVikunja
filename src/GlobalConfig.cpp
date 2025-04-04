#include "GlobalConfig.h"
#include <SD.h>
#include <SPI.h>
const char *filename = "/config.json";

Configuration::Configuration()
{
    _json_doc = new DynamicJsonDocument(16384);

}

Configuration::~Configuration()
{
    _json_doc->clear();
}

void Configuration::LoadConfiguration()
{
    File file = SD.open(filename);
    log_e("Loading configuration!");
    DeserializationError error = deserializeJson(*_json_doc,file.readString());
    if(error)
    {
        log_e("Could not load configuration file!");
    }

    screenHeight = (*_json_doc)["screenHeight"]|960;
    screenWidth = (*_json_doc)["screenWidth"]|540;

    headerHeight = (*_json_doc)["headerHeight"]|60;
    numberOfHeaderElements = (*_json_doc)["numberOfHeaderElements"]|3;
    numberOfLines = (*_json_doc)["numberOfLines"]|10;

    sleepTime = (*_json_doc)["sleepTime"]|12; //12 hours
    sleepTime *= 60*60;
    numberOfTimesToWakePerDay = (*_json_doc)["numberOfTimesToWakePerDay"]|2;
    batteryMax = (*_json_doc)["batteryMax"]|4230;
    batteryMin = (*_json_doc)["batteryMin"]|3300;
    ssid = (*_json_doc)["ssid"]|"";
    wifiPassword = (*_json_doc)["wifiPassword"]|"";

    showDone = (*_json_doc)["showDone"]|false;

    url = (*_json_doc)["url"]|"";
    timeZone = (*_json_doc)["timeZone"]|0;
    toDoUser = (*_json_doc)["toDoUser"]|"M5Paper";
    toDoPassword = (*_json_doc)["toDoPassword"]|"M5PaperPassword";
    mainProjectName = (*_json_doc)["mainProjectName"]|"M5Paper";
    secondaryProjectName = (*_json_doc)["secondaryProjectName"]|"Groceries";
    tertiaryProjectName = (*_json_doc)["tertiaryProjectName"]|"GeneralPurchases";
    timeToSleep = (*_json_doc)["timeToSleep"]|2*60*1000;
    file.close();
}

int Configuration::getScreenHeight()
{
    return screenHeight;
}
int Configuration::getScreenWidth()
{
    return screenWidth;
}
int Configuration::getHeaderHeight()
{
    return headerHeight;
}
int Configuration::getNumberOfHeaderElements()
{
    return numberOfHeaderElements;
}
int Configuration::getHeaderWidth()
{
    return screenWidth/numberOfHeaderElements;
}
int Configuration::getNumberOfLines()
{
    return numberOfLines;
}
int Configuration::getLineHeight()
{
    return (screenHeight - headerHeight)/numberOfLines;
}
int Configuration::getSleepTime()
{
    return sleepTime;
}
int Configuration::getNumberOfTimesToWakePerDay()
{
    return numberOfTimesToWakePerDay;
}
int Configuration::getBatteryMax()
{
    return batteryMax;
}
int Configuration::getBatteryMin()
{
    return batteryMin;
}

String Configuration::getSsid()
{
    return ssid;
}
String Configuration::getWifiPassword()
{
    return wifiPassword;
}
String Configuration::getUrl()
{
    return url;
}
int Configuration::getTimezone()
{
    return timeZone;
}
String Configuration::getToDoUser()
{
    return toDoUser;
}
String Configuration::getToDoPassword()
{
    return toDoPassword;
}
String Configuration::getMainProjectName()
{
    return mainProjectName;
}
String Configuration::getSecondaryProjectName()
{
    return secondaryProjectName;
}
String Configuration::getTertiaryProjectName()
{
    return tertiaryProjectName;
}

bool Configuration::getShowDone()
{
    return showDone;
}

uint32_t Configuration::getTimeToSleep()
{
    return timeToSleep;
}
