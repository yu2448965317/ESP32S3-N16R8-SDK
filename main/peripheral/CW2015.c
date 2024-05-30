#include "lvgl_gui.h"
const char *TAG = "CW2015";
float bat_voltage  =  0; //电池电压
float bat_percentage  =  0; //电池电量百分比
uint16_t remain_minutes  =  0; //电池预估剩余续航，需要插入电池使用一段时间才准确
void CW2015_init()
{
    uint8_t data[] = {0x0A, 0b00110000};
    uint8_t version;
    ESP_ERROR_CHECK(i2c_master_write_to_device(I2C_NUM, CW2015_ADDR, data, sizeof(data), pdMS_TO_TICKS(1000)));
    vTaskDelay(pdMS_TO_TICKS(100));
    ESP_ERROR_CHECK(i2c_master_write_read_device(I2C_NUM, CW2015_ADDR, (uint8_t[]){0x00}, 1, &version, 1, pdMS_TO_TICKS(1000)));
    ESP_LOGW(TAG,"Version:%d",version);
}
void print_CW2015_data()
{
    uint8_t adc_value[2];
    uint8_t percentage_value[2];
    uint8_t remain_value[2];   
    check_usb_in();                                                                         //获取USB输入状态
    ESP_ERROR_CHECK(i2c_master_write_read_device(I2C_NUM, CW2015_ADDR, (uint8_t[]){0x02}, 1, &adc_value, 2, pdMS_TO_TICKS(1000)));
    ESP_ERROR_CHECK(i2c_master_write_read_device(I2C_NUM, CW2015_ADDR, (uint8_t[]){0x04}, 1, &percentage_value, 2, pdMS_TO_TICKS(1000)));
    ESP_ERROR_CHECK(i2c_master_write_read_device(I2C_NUM, CW2015_ADDR, (uint8_t[]){0x06}, 1, &remain_value, 2, pdMS_TO_TICKS(1000)));
    bat_voltage=((adc_value[0]*256.0+(float)adc_value[1])*305.0)/1000000.0;
    bat_percentage = percentage_value[0]+percentage_value[1]/256.0;
    remain_minutes = remain_value[1]+((remain_value[0]&0x3F)<<8);
    ESP_LOGI(TAG,"锂电电压:%.3fV  电量百分比:%.3f%%  预计续航:%d分钟",bat_voltage,bat_percentage,remain_minutes);
}