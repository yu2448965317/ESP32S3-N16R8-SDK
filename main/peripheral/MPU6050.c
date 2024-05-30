#include "lvgl_gui.h"
static const char *TAG = "MPU6050";
static mpu6050_handle_t mpu6050 = NULL;
mpu6050_acce_value_t acce;//MPU6050加速度值
mpu6050_gyro_value_t gyro;//MPU6050角度值
mpu6050_temp_value_t temp;//MPU6050温度
void i2c_sensor_mpu6050_init(void)
{
    mpu6050 = mpu6050_create(I2C_NUM, MPU6050_I2C_ADDRESS);
    ESP_ERROR_CHECK(mpu6050_config(mpu6050, ACCE_FS_4G, GYRO_FS_500DPS));
    ESP_ERROR_CHECK(mpu6050_wake_up(mpu6050));
}
void MPU6050_init()
{
    uint8_t mpu6050_deviceid;
    i2c_sensor_mpu6050_init();
    ESP_ERROR_CHECK( mpu6050_get_deviceid(mpu6050, &mpu6050_deviceid));
}
void print_MPU6050_data()
{

        ESP_ERROR_CHECK(mpu6050_get_acce(mpu6050, &acce));
        ESP_ERROR_CHECK(mpu6050_get_gyro(mpu6050, &gyro));
        ESP_ERROR_CHECK(mpu6050_get_temp(mpu6050, &temp));
        ESP_LOGI(TAG, "acce_x:%.2f, acce_y:%.2f, acce_z:%.2f,gyro_x:%.2f, gyro_y:%.2f, gyro_z:%.2f   温度:%.2f ",
                 acce.acce_x, acce.acce_y, acce.acce_z,gyro.gyro_x, gyro.gyro_y, gyro.gyro_z,temp.temp);
}
