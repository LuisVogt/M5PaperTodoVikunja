#include "ListGUI.h"
#include "GlobalConfig.h"
#include <M5EPD.h>

ListGUI::ListGUI()
{
    
}

ListGUI::~ListGUI()
{

}

void CallCompleteTask(Task *task)
{
    toDoAPI.CompleteTask(task);
}

void ListGUI::Setup()
{
    //toDoAPI.Begin(DEFAULT_TIME_ZONE);
    delay(100);
    toDoAPI.LogIn();
    delay(100);
    toDoAPI.GetProjectIds();
    delay(100);
}

void ListGUI::CreateButtons()
{
    std::list<Task> Tasks;
    std::list<Task> TasksSecondary;
    std::list<Task> TasksTertiary;
    //Make this a list

    rtc_date_t RTCdate;
    M5.RTC.getDate(&RTCdate);

    log_d("Creating buttons");
    buttons.clear();
    Tasks = toDoAPI.GetAllTasks(0);
    TasksSecondary = toDoAPI.GetAllTasks(1);
    TasksTertiary = toDoAPI.GetAllTasks(2);
    //separate functions to get tasks and create buttons
    int i = 0;
    Task task;
    buttons.emplace_back(-1,configuration.getHeaderWidth()*i++,0,configuration.getHeaderWidth(),configuration.getHeaderHeight(),GetClockString()+"-"+GetNextWakeUpTime(),"",task,CL_DATUM);
    buttons.emplace_back(-1,configuration.getHeaderWidth()*i++,0,configuration.getHeaderWidth(),configuration.getHeaderHeight(),"\\"+String(TasksSecondary.size())+"/","",task,CC_DATUM,configuration.getHeaderWidth()/2);
    buttons.emplace_back(-1,configuration.getHeaderWidth()*i++,0,configuration.getHeaderWidth(),configuration.getHeaderHeight(),String(TasksTertiary.size())+"$","",task,CC_DATUM,configuration.getHeaderWidth()/2);
    buttons.emplace_back(-1,configuration.getHeaderWidth()*i++,0,configuration.getHeaderWidth(),configuration.getHeaderHeight(),GetBatteryString(),"",task,CR_DATUM,configuration.getHeaderWidth()-16);
    i = 0;
    for(std::list<Task>::iterator taskPos = Tasks.begin(); taskPos != Tasks.end() &&i<configuration.getNumberOfLines(); taskPos++)
    {
        //Check if it is after StartDate to display
        log_d("id: %d, dueDate: %d-%d-%d, currentDate: %d-%d-%d",taskPos->id,taskPos->due_date.tm_year,taskPos->due_date.tm_mon,taskPos->due_date.tm_mday,RTCdate.year,RTCdate.mon,RTCdate.day);
        if(isDateAfterClock(taskPos->start_date_stamp))
        {
            buttons.emplace_back(taskPos->id,0, configuration.getHeaderHeight() + configuration.getLineHeight() * i, configuration.getScreenWidth(), configuration.getLineHeight(),taskPos->title,GetNextDueDateString(taskPos->due_date, taskPos->due_date_local), *taskPos);
            buttons.back().SetPressedFunction(CallCompleteTask);
            i++;
        }
    }
}

int ListGUI::getBatteryLevel()
{
    uint32_t vol = M5.getBatteryVoltage();
    if (vol < configuration.getBatteryMin()) {
        vol = configuration.getBatteryMin();
    } else if (vol > configuration.getBatteryMax()) {
        vol = configuration.getBatteryMax();
    }
    float battery = (float)(vol - configuration.getBatteryMin()) / (float)(configuration.getBatteryMax() - configuration.getBatteryMin());
    if (battery <= 0.01) {
        battery = 0.01;
    }
    if (battery > 1) {
        battery = 1;
    }
    return (int)(battery * 100);
}

String ListGUI::GetClockString()
{
    char buf[20];

    rtc_time_t RTCtime;
    M5.RTC.getTime(&RTCtime);

    sprintf(buf, "%2d:%02d", RTCtime.hour, RTCtime.min);
    return (String)buf;
}

String ListGUI::GetNextDueDateString(tm time, time_t timeStamp)
{
    if(timeStamp<0)
        return "";
    rtc_date_t RTCdate;
    rtc_time_t RTCtime;
    M5.RTC.getDate(&RTCdate);
    M5.RTC.getTime(&RTCtime);
    
    tm currentTm;
    currentTm.tm_year = RTCdate.year-1900;
    currentTm.tm_mon = RTCdate.mon-1;
    currentTm.tm_mday = RTCdate.day;
    currentTm.tm_hour = RTCtime.hour;
    currentTm.tm_min = RTCtime.min;
    currentTm.tm_sec = RTCtime.sec;

    time_t currentStamp = mktime(&currentTm);
    log_d("Getting nextDueDate: %d : %d (now)",timeStamp,currentStamp);
    long timeDifference = (timeStamp-currentStamp);
    String overdue = timeDifference < 0?"!!":"";
    String returnString;
    if(timeDifference/(60*60*24) != 0)
    {
        returnString = (String)abs(timeDifference/(60*60*24))+"d";
        return overdue + returnString + overdue;
    }

    return overdue + (String)(time.tm_hour) + "h" + overdue;
}

bool ListGUI::isDateAtOrBeforeClock(time_t timeStamp)
{
    rtc_date_t RTCdate;
    rtc_time_t RTCtime;
    M5.RTC.getDate(&RTCdate);
    M5.RTC.getTime(&RTCtime);
    
    tm currentTm;
    currentTm.tm_year = RTCdate.year-1900;
    currentTm.tm_mon = RTCdate.mon-1;
    currentTm.tm_mday = RTCdate.day;
    currentTm.tm_hour = RTCtime.hour;
    currentTm.tm_min = RTCtime.min;
    currentTm.tm_sec = RTCtime.sec;

    time_t currentStamp = mktime(&currentTm);
    
    return currentStamp + 60*60*24*30 > timeStamp;
}

bool ListGUI::isDateAfterClock(time_t timeStamp)
{
    rtc_date_t RTCdate;
    rtc_time_t RTCtime;
    M5.RTC.getDate(&RTCdate);
    M5.RTC.getTime(&RTCtime);
    
    tm currentTm;
    currentTm.tm_year = RTCdate.year-1900;
    currentTm.tm_mon = RTCdate.mon-1;
    currentTm.tm_mday = RTCdate.day;
    currentTm.tm_hour = RTCtime.hour;
    currentTm.tm_min = RTCtime.min;
    currentTm.tm_sec = RTCtime.sec;

    time_t currentStamp = mktime(&currentTm);
    
    return currentStamp > timeStamp;
}

String ListGUI::GetNextWakeUpTime()
{
    char buf[20];

    rtc_time_t RTCtime;
    M5.RTC.getTime(&RTCtime);

    int interval = 24/configuration.getNumberOfTimesToWakePerDay();
    int NextTime = (1+RTCtime.hour/interval)*interval;
    NextTime = NextTime>=24? 0: NextTime;

    sprintf(buf, "%2dh", NextTime);
    return (String)buf;
}

String ListGUI::GetHumidityString()
{
    M5.SHT30.Begin();
    M5.SHT30.UpdateData();

    char buf[20];
    sprintf(buf, "%d° %d%%", (int)M5.SHT30.GetTemperature(), (int)M5.SHT30.GetAbsHumidity());
    return (String)buf;
}

String ListGUI::GetBatteryString()
{
    char buf[20];
    sprintf(buf, "%d%%", getBatteryLevel());

    return (String)buf;
}

void ListGUI::Draw(M5EPD_Canvas &canvas)
{
    canvas.fillCanvas(0);
    for(std::list<ButtonGUI>::iterator button = buttons.begin();button!=buttons.end();button++)
    {
        button->Draw(canvas);
        //delay(200);
    }
    canvas.pushCanvas(0, 0, UPDATE_MODE_GC16);
}

void ListGUI::Update(M5EPD_Canvas &canvas)
{
    static int posX = 0;
    static int posY = 0;
    bool buttonsUpdated = false;

    if(M5.TP.avaliable())
    {
        M5.TP.update();
        bool isFingerUp = M5.TP.isFingerUp();
        if(isFingerUp || posX != M5.TP.readFingerX(0) || posY != M5.TP.readFingerY(0))
        {
            posX = M5.TP.readFingerX(0);
            posY = M5.TP.readFingerY(0);
            if(!isFingerUp)
            {
                log_d("fingering posX: %d posY: %d", posX, posY);
                for(std::list<ButtonGUI>::iterator button = buttons.begin();button!=buttons.end();button++)
                {
                    if(button->isInBox(posX, posY))
                    {
                        buttonsUpdated = true;
                        log_d("button fingered posX: %d posY: %d", posX, posY);
                    }
                }
            }
        }
    }
    if(buttonsUpdated)
    {
        CreateButtons();
        M5.EPD.UpdateFull(UPDATE_MODE_DU4);
        Draw(canvas);
    }
    M5.TP.flush();
}

