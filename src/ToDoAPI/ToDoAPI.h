#pragma once
#define ARDUINOJSON_DECODE_UNICODE 1
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <List>
#include <time.h>
#include <M5EPD.h>

// Calls to the target API.

#define DEFAULT_TIME_ZONE -3
#define DEFAULT_URL       "http://192.168.0.3:3456/api/v1/"
#define DEFAULT_USER      "M5Paper"
#define DEFAULT_PASSWORD  "M5PaperPassword"
#define DEFAULT_PROJECT_NAME "M5Paper"
#define DEFAULT_SHOW_DONE false
typedef struct 
{
    int id;
    String title;
    struct tm due_date;
    time_t due_date_local;
    time_t start_date_stamp;
    long repeat_after = 0;
    bool done;
}Task;

class ToDoAPI
{
    public:
        ToDoAPI(int i);
        ~ToDoAPI();
        esp_err_t Begin(int8_t timezone);
        esp_err_t LogIn();
        esp_err_t GetProjectIds();
        esp_err_t GetProjectId(String title, String& id);
        esp_err_t ClearTasks();
        std::list<Task> GetAllTasks(int level);
        std::list<Task> GetAllTasks(String id);
        esp_err_t CompleteTask(Task *task);
        static void CompleteTask(int id, String title);
        esp_err_t syncTime(int8_t timezone);
        void convertTime(String datetime, struct tm *target_time, time_t *target_stamp);
    private:
        esp_err_t httpRequestResult(int httpResponseCode, HTTPClient &http, String &result);
        esp_err_t deserializeResponse(String result, DynamicJsonDocument *filter = NULL);
        std::vector<String> split(String src, String delim);

    public:
        std::list<Task> Tasks;
    private:
        static String token;
        String mainProjectId;
        String secondaryProjectId;
        String tertiaryProjectId;

        time_t _time_stamp;
        struct tm _time_info;
        DynamicJsonDocument *_json_doc;
        String _err_info;
        
};

extern ToDoAPI toDoAPI;