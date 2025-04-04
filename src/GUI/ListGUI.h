#pragma once
#include "ButtonGUI.h"
#include "ToDoAPI/ToDoAPI.h"
#include <List>

class ListGUI
{
    public:
        ListGUI();
        ~ListGUI();
        void Setup();
        void CreateButtons();
        void Draw(M5EPD_Canvas &canvas);
        void Update(M5EPD_Canvas &canvas);
    private:
        std::list<ButtonGUI> buttons;
        bool isDateAtOrBeforeClock(time_t time);
        bool isDateAfterClock(time_t time);
        String GetClockString();   
        String GetNextDueDateString(tm time, time_t timeStamp);     
        String GetNextWakeUpTime();
        String GetHumidityString();
        String GetBatteryString();
        int getBatteryLevel();
};