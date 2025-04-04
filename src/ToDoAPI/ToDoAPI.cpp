#include "ToDoAPI.h"
#include "GlobalConfig.h"

ToDoAPI::ToDoAPI(int i)
{
    _json_doc = new DynamicJsonDocument(16384);
}

ToDoAPI::~ToDoAPI()
{
    ClearTasks();
    _json_doc->clear();
}

esp_err_t ToDoAPI::Begin(int8_t timezone)
{
    _timezone = timezone;
    for (int retry = 0; retry < 3; retry++)
    {
        if (syncTime(timezone) == ESP_OK)
        {
            break;
        }
    }
    return ESP_OK;
}

String ToDoAPI::token = "";

esp_err_t ToDoAPI::LogIn()
{
    HTTPClient http;
    String url = configuration.getUrl() + "login";

    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    String longToken = "true";
    String data = "{\"long_token\": "+longToken+"," +
    "\"password\":\"" + configuration.getToDoPassword() + "\"," +  
    "\"username\": \"" + configuration.getToDoUser() + 
    "\"}";

    int httpResponseCode = http.POST(data);

    String result;
    result = http.getString();

    http.end();
    
    if(deserializeResponse(result)!=ESP_OK)
    {
        return ESP_FAIL;
    }
    token = (*_json_doc)["token"].as<String>();
    log_d("Token0: %s", result);
    log_d("Token: %s", (*_json_doc)["token"]);
    log_d("Token2: %s", token);
    return ESP_OK;
}


esp_err_t ToDoAPI::GetProjectIds()
{
    GetProjectId(configuration.getMainProjectName(),mainProjectId);
    log_d("Project 1: %s",configuration.getMainProjectName());
    GetProjectId(configuration.getSecondaryProjectName(),secondaryProjectId);
    log_d("Project 2: %s",configuration.getSecondaryProjectName());
    GetProjectId(configuration.getTertiaryProjectName(),tertiaryProjectId);
    log_d("Project 3: %s",configuration.getTertiaryProjectName());
    return ESP_OK;
}

esp_err_t ToDoAPI::GetProjectId(String title, String& id)
{
    HTTPClient http;
    String data = "?s="+title;
    String url = configuration.getUrl() + "projects" + data;
    http.setAuthorizationType("Bearer");
    http.setAuthorization(token.c_str());

    http.begin(url);

    int httpResponseCode = http.GET();

    String result;
    if(httpRequestResult(httpResponseCode,http,result)!=ESP_OK)
    {
        return ESP_FAIL;
    }
    http.end();
    
    if(deserializeResponse(result)!=ESP_OK)
    {
        return ESP_FAIL;
    }

    //JsonArray array = (*_json_doc).as<JsonArray>();
    //log_d("array created %s",array.size());

    //char * buf[title.length()];
    id = (*_json_doc)[0]["id"].as<String>();
    log_d("title: %s, id:%s",title,id);
    return ESP_OK;
}

esp_err_t ToDoAPI::ClearTasks()
{
    Tasks.clear();
    return ESP_OK;
}

std::list<Task> ToDoAPI::GetAllTasks(int level)
{
    switch (level)
    {
        case 0:
            return GetAllTasks(mainProjectId);
        case 1:
            return GetAllTasks(secondaryProjectId);
        case 2:
            return GetAllTasks(tertiaryProjectId);
    }
    return std::list<Task>();
}

std::list<Task> ToDoAPI::GetAllTasks(String id)
{
    Tasks.clear();

    HTTPClient http;
    int view = id.toInt()*4 - 3;
    //String data = "?filter_by[]=done&filter_value[]=false&filter_comparator[]=equals&filter_concat=and";
    String data = "?sort_by[]=position&order_by[]=desc&filter_by[]=done&filter_value[]=false&filter_comparator[]=equals&filter_concat=and&page=1";
    String url = configuration.getUrl() + "projects/"+id+"/views/"+ (String)view+ "/tasks"+data;//; + showDone? "" : data;
    http.setAuthorizationType("Bearer");
    http.setAuthorization(token.c_str());
    
    http.begin(url);

    int httpResponseCode = http.GET();
    String result;
    if(httpRequestResult(httpResponseCode,http,result)!=ESP_OK)
    {
        return Tasks;
    }
    http.end();

    log_d("Preserialize");
    if(deserializeResponse(result)!=ESP_OK)
    {
        return Tasks;
    }
    log_d("Postserialize");

    log_d("task clearing");

    JsonArray array = (*_json_doc).as<JsonArray>();
    //log_d("array created %s",array.size());

    for(JsonVariant v : array) {
        if(!configuration.getShowDone() && v["done"])
        {
            continue;
        }
        Task task;
        task.id  = v["id"].as<int>();
        task.title = v["title"].as<String>();
        task.done  = v["done"].as<bool>();
        task.repeat_after = v["repeat_after"].as<long>();

        tm dueDate;
        time_t dueDateLocal;

        if(v.containsKey("due_date"))
        {
            convertTime(v["due_date"].as<String>(), &dueDate, &dueDateLocal);
        }        
        task.due_date = dueDate;
        task.due_date.tm_hour += (DEFAULT_TIME_ZONE);
        task.due_date_local = dueDateLocal + (DEFAULT_TIME_ZONE * 3600);

        tm startDate;
        time_t startDateStamp;
        if(v.containsKey("start_date"))
        {
            convertTime(v["start_date"].as<String>(), &startDate, &startDateStamp);
        }
        //task.start_date = startDateStamp;
        //task.start_date.tm_hour += (DEFAULT_TIME_ZONE);
        task.start_date_stamp = startDateStamp + (DEFAULT_TIME_ZONE * 3600);

        //log_d("Serializing timestamp is %d",dueDateLocal);
        log_d("Task title: %s", task.title);
        log_d("Task Id: %d", task.id);
        Tasks.push_back(task);
    }
    log_d("NumberofTasks: %d", Tasks.size());
    return Tasks;
}

esp_err_t ToDoAPI::CompleteTask(Task *task)
{
    task->done = true;
    HTTPClient http;
    String url = configuration.getUrl() + "tasks/"+task->id;

    log_d("TaskIdToComplete: %d", task->id);
    log_d("TaskTitleToComplete: %s", task->title);

    http.setAuthorizationType("Bearer");
    http.setAuthorization(token.c_str());

    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    String repeatAfter = task->repeat_after>0? + ",\"repeat_after\":"+(String)(task->repeat_after):"";
    String data = "{\"done\":true" + repeatAfter + "}";
    log_d("DataToSend: %s", ((String)(task->done)).c_str());

    int httpResponseCode = http.POST(data);
    String result;
    if(httpRequestResult(httpResponseCode,http,result)!=ESP_OK)
    {
        return ESP_FAIL;
    }
    http.end();
    return ESP_OK;
}

void ToDoAPI::CompleteTask(int id, String title)
{
    bool done = true;
    HTTPClient http;
    String url = configuration.getUrl() + "tasks/"+id;

    log_d("TaskIdToComplete: %d", id);
    log_d("TaskTitleToComplete: %s", title);

    http.setAuthorizationType("Bearer");
    http.setAuthorization(token.c_str());


    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    String data = "{\"done\":true}";
    int httpResponseCode = http.POST(data);
    String result;
    http.end();
}

///////////

esp_err_t ToDoAPI::syncTime(int8_t timezone)
{
    //const char *ntpServer = "time.cloudflare.com";
    const char *ntpServer2 = "a.st1.ntp.br";
    const char *ntpServer3 = "time.nist.gov";
    configTime(timezone * 3600, 0, /*ntpServer,*/ntpServer2,ntpServer3);
    log_d("ntp'd");
    if (getLocalTime(&_time_info))
    {
        log_d("ntp'd2");
        rtc_time_t time_struct;
        time_struct.hour = _time_info.tm_hour;
        time_struct.min = _time_info.tm_min;
        time_struct.sec = _time_info.tm_sec;
        log_d("Time Synced %d:%02d",time_struct.hour, time_struct.min);
        M5.RTC.setTime(&time_struct);
        rtc_date_t date_struct;
        date_struct.week = _time_info.tm_wday;
        date_struct.mon = _time_info.tm_mon + 1;
        date_struct.day = _time_info.tm_mday;
        date_struct.year = _time_info.tm_year + 1900;
        M5.RTC.setDate(&date_struct);
        return ESP_OK;
    }
    log_e("Time sync failed");
    return ESP_FAIL;
}

esp_err_t ToDoAPI::httpRequestResult(int httpResponseCode, HTTPClient &http, String &result)
{
    if (httpResponseCode != 200)
    {
        if(httpResponseCode==400)
        {
            String result2 = http.getString();
            deserializeResponse(result2);
            log_e("Error occurred. Description: %s", (*_json_doc)["message"].as<String>());
        }
        log_e("HTTP Request failed. code = %d. payload = %s", httpResponseCode, result);
        _err_info = "HTTP request failed. code = " + String(httpResponseCode);
        return ESP_FAIL;
    }
    result = http.getString();
    return ESP_OK;
}

esp_err_t ToDoAPI::deserializeResponse(String result, DynamicJsonDocument *filter)
{
    DeserializationError err;
    if(filter == nullptr)
        err = deserializeJson((*_json_doc), result);
    else
        err = deserializeJson((*_json_doc), result, DeserializationOption::Filter(*filter));
    if (err)
    {
        log_e("DeserializeJson failed: %s", err.c_str());
        _err_info = "DeserializeJson failed: " + String(err.c_str());
        return ESP_FAIL;
    }
    log_d("json doc usage: %d/%d", (*_json_doc).memoryUsage(), (*_json_doc).capacity());
    if ((*_json_doc).containsKey("error"))
    {
        log_e("Error occurred. Description: %s", (*_json_doc)["error_description"].as<String>().c_str());
        _err_info = "Error occurred. Description: " + (*_json_doc)["error_description"].as<String>();
        return ESP_FAIL;
    }
    return ESP_OK;
}

void ToDoAPI::convertTime(String datetime, struct tm *target_time, time_t *target_stamp)
{
    // "2020-11-04T08:48:00.0940839Z"
    struct tm time;
    std::vector<String> buf;
    String temp;

    temp = datetime.substring(0, 10);
    buf = split(temp, "-");
    time.tm_year = buf[0].toInt() - 1900;
    time.tm_mon = buf[1].toInt() - 1;
    time.tm_mday = buf[2].toInt();
    temp = datetime.substring(11, 19);
    buf = split(temp, ":");
    time.tm_hour = buf[0].toInt();
    time.tm_min = buf[1].toInt();
    time.tm_sec = buf[2].toInt();

    time_t stamp = mktime(&time);
    log_d("timeStampIs %d",stamp);

    stamp += _timezone * 3600;
    *target_stamp = stamp;

    time = *localtime(&stamp);
    time.tm_year += 1900;
    time.tm_mon += 1;
    
    *target_time = time;
}

std::vector<String> ToDoAPI::split(String src, String delim)
{
    std::vector<String> buf;
    int32_t flag = 0, pst = 0;
    int32_t len = delim.length();

    while (1)
    {
        flag = src.indexOf(delim, flag);
        if (flag < 0)
            break;
        buf.push_back(src.substring(pst, flag));
        pst = flag + len;
        flag += len;
    }

    if (pst < src.length())
    {
        buf.push_back(src.substring(pst, src.length()));
    }

    return buf;
}
ToDoAPI toDoAPI(1);