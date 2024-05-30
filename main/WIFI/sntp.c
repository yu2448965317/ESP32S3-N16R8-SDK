
#include "lvgl_gui.h"
static const char *TAG = "SNTP";
#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 48
#endif

RTC_DATA_ATTR static int boot_count = 0;
struct tm timeinfo;
time_t now; 
static void obtain_time(void);
#ifdef CONFIG_SNTP_TIME_SYNC_METHOD_CUSTOM
void sntp_sync_time(struct timeval *tv)
{
   settimeofday(tv, NULL);
   ESP_LOGI(TAG, "Time is synchronized from custom code");
   sntp_set_sync_status(SNTP_SYNC_STATUS_COMPLETED);
}
#endif

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

static void obtain_time(void)
{
    initialize_sntp();
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 15;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    time(&now);
    localtime_r(&now, &timeinfo);
}

void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_stop();
    sntp_enabled();
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_setservername(3, "cn.pool.ntp.org"); // 中国区NTP服务的虚拟集群
    sntp_setservername(1, "210.72.145.44"); // 国家授时中心服务器 IP 地址
    sntp_setservername(2, "ntp1.aliyun.com");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_init();
}
bool test_sntp(void)
{
    setenv("TZ", "CST-8", 1);
    tzset();
    time(&now);
    localtime_r(&now, &timeinfo);
    if (timeinfo.tm_year < (2016 - 1900)) {
        ESP_LOGW(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
         if(ui_TextArea1!=NULL)
         {
            lv_textarea_add_text(ui_TextArea1,"SNTP:正在尝试获取时间...\n");
             lv_label_set_text(ui_Header_Time, "正在尝试获取时间...");
        }
        obtain_time();
        time(&now);
        return 0;
    }
    else 
    {
        char text[100];
        ESP_LOGI(TAG, "北京时间:%d年%02d月%02d日,%02d:%02d:%02d 周%d",timeinfo.tm_year + 1900,timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,timeinfo.tm_wday);
        sprintf(text,"时间:%d年%02d月%02d日,%02d:%02d:%02d 周%d\n",timeinfo.tm_year + 1900,timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,timeinfo.tm_wday);
        if(ui_TextArea1!=NULL)
        {
            lv_textarea_add_text(ui_TextArea1,text);
            lv_label_set_text(ui_Header_Time,text+7);
        }
        return 1;
    } 
}
