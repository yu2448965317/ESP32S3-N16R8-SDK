#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "protocol_examples_common.h"
#include "esp_sntp.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_smartconfig.h"
#include "driver/gptimer.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lvgl.h"
#include "driver/ledc.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "demos/lv_demos.h"
#include "driver/i2c.h"
#include "esp_lcd_touch_gt911.h"
#include "esp_vfs_fat.h"
#include "unity.h"
#include "mpu6050.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "esp_err.h"
#include "esp_log.h"
#include "AS5600.h"
#include "gpio_define.h"
#include "foc.h"
//#include "ui.h"

#define  CONFIG_FILE     "config"  //NVS配置文件
#define  MAX_BRIGHT   1023     //滑动条可设置最大亮度
#define  MIN_BRIGHT   1        //滑动条可最小亮度 设置0表示可熄灭
#define  DEFAULT_BRIGHT  512   //默认亮度 
/*电流计IIC地址*/
#define CW2015_ADDR  0x62

/*角度编码器寄存器地址和定义*/ 
#define As5600_Addr 0x36
#define RawAngle_Addr 0x0C
#define I2C_WRITE_MODE      0
#define I2C_READ_MODE       1
#define ACK                 0   
#define NACK                1                    
#define ACK_CHECK_ENABLE    1
#define ACK_CHECK_DISABLE   0

/*LCD部分参数定义*/
#define LINES 50                        // 调整LCD 缓存Buffer大小,一定程度上影响刷新率 
#define LCD_PIXEL_CLOCK_HZ (20 * 1000 * 1000)
#define LCD_BK_LIGHT_ON_LEVEL 1
#define LCD_BK_LIGHT_OFF_LEVEL !LCD_BK_LIGHT_ON_LEVEL
#define LCD_H_RES 800    //分辨率
#define LCD_V_RES 480
#define LCD_CMD_BITS 8
#define LCD_PARAM_BITS 8
#define LVGL_TICK_PERIOD_MS 2
#define PSRAM_DATA_ALIGNMENT 64

extern nvs_handle_t my_handle;
extern float bat_voltage; //电池电压
extern float bat_percentage; //电池电量百分比
extern uint16_t remain_minutes; //电池预估剩余续航，需要插入电池使用一段时间才准确
extern mpu6050_acce_value_t acce;//MPU6050加速度值
extern mpu6050_gyro_value_t gyro;//MPU6050角度值
extern mpu6050_temp_value_t temp;//MPU6050温度

void guiTask(void *pvParameter);
void initialise_wifi(void);
void initialize_sntp(void);
bool test_sntp(void);
void MPU6050_init();
void print_MPU6050_data();
void CW2015_init();
void print_CW2015_data();
void gpio_adc_init();
int check_usb_in();
esp_err_t user_init_sdcard();
void save_config(const char * key , int32_t value );
