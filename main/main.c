
#include "lvgl_gui.h"

static const char *TAG = "main";
nvs_handle_t my_handle;
PID p_vel_p,p_vel_i,p_ang_p,p_ang_d,out_vel;
void read_foc_info()
{
    int32_t p_v_p,p_v_i,out_v_m,p_a_p,p_a_d;
    p_vel_p.key = "pid_vel_P";
    p_vel_p.value = &pid_vel_P;
    p_vel_i.key = "pid_vel_I";
    p_vel_i.value = &pid_vel_I;
    p_ang_p.key = "pid_ang_P";
    p_ang_p.value = &pid_ang_P;
    p_ang_d.key = "pid_ang_D";
    p_ang_d.value = &pid_ang_D;
    out_vel.key = "output_vel_ramp";
    out_vel.value = &output_vel_ramp;
    esp_err_t err;
    err = nvs_open(CONFIG_FILE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
    printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } 
    else 
    {
    printf("Opening NVS handle successful!\n");
    }
    err = nvs_get_i32(my_handle, p_vel_p.key, &p_v_p);
    switch (err) {
        case ESP_OK:
            *(p_vel_p.value) = (float)p_v_p/ (float)100;
            printf("pid_vel_P = %.2f\n",pid_vel_P);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("Default set pid_vel_P to %.2f\n",0.2);
            nvs_set_i32(my_handle, p_vel_p.key, 20);
            nvs_commit(my_handle);
            break;
        default :
            printf("Error (%s) reading!\n", esp_err_to_name(err));
    }
    err = nvs_get_i32(my_handle, p_vel_i.key, &p_v_i);
    switch (err) {
        case ESP_OK:
           *(p_vel_i.value) =  (float)p_v_i / (float)100;
            printf("pid_vel_I = %.2f\n", pid_vel_I );
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("Default set pid_vel_I to %.2f\n",10.00);
            nvs_set_i32(my_handle, p_vel_i.key, 1000);
            nvs_commit(my_handle);
            break;
        default :
            printf("Error (%s) reading!\n", esp_err_to_name(err));
    }
    err = nvs_get_i32(my_handle, out_vel.key, &out_v_m);
    switch (err) {
        case ESP_OK:
            *(out_vel.value) =  (float)out_v_m / (float)100;
            printf("output_vel_ramp = %.2f\n",output_vel_ramp);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("Default set output_vel_ramp to %.2f\n",50.0);
            nvs_set_i32(my_handle, out_vel.key, 5000);
            nvs_commit(my_handle);
            break;
        default :
            printf("Error (%s) reading!\n", esp_err_to_name(err));
    }
    err = nvs_get_i32(my_handle, p_ang_p.key, &p_a_p);
    switch (err) {
        case ESP_OK:
            *(p_ang_p.value)=  (float)p_a_p/ (float)100;
            printf("pid_ang_P = %.2f\n",pid_ang_P);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("Default set pid_ang_P to %.2f\n",5.0);
            nvs_set_i32(my_handle, p_ang_p.key, 500);
            nvs_commit(my_handle);
            break;
        default :
            printf("Error (%s) reading!\n", esp_err_to_name(err));
    }   
    err = nvs_get_i32(my_handle, p_ang_d.key, &p_a_d);
    switch (err) {
        case ESP_OK:
             *(p_ang_d.value) =  (float)p_a_d/ (float)100;
            printf("pid_ang_D = %.2f\n",pid_ang_D);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("Default set pid_ang_D to %.2f\n",0.8);
            nvs_set_i32(my_handle, p_ang_d.key, 80);
            nvs_commit(my_handle);
            break;
        default :
            printf("Error (%s) reading!\n", esp_err_to_name(err));
    } 
    nvs_close(my_handle);   
}
void save_config(const char * key , int32_t value )
{
    esp_err_t err;
    err = nvs_open(CONFIG_FILE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
    printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } 
    else 
    {
    printf("Opening NVS handle successful!\n");
    }
    nvs_set_i32(my_handle, key, value);
    nvs_commit(my_handle);
    nvs_close(my_handle);   
}
static void print_rtos_status(void *pvParameters)
{
    // char buf[1024];
    while (1)
    {
        size_t size = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
        size_t size_internal = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
        ESP_LOGW(TAG, "%d, %d", size, size_internal);
        // vTaskList(buf);
        // ESP_LOGI(TAG, "\n%s", buf);
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
}
void i2c_init()
{
    ESP_LOGI(TAG, "Initialize I2C");
    const i2c_config_t i2c_conf = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = I2C_SDA,
    .scl_io_num = I2C_SCL,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = 400000,
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM, &i2c_conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM, i2c_conf.mode, 0, 0, 0));
}
void foc_loop()
{
    voltage_power_supply = 12;  // 电机驱动输入电压
	voltage_limit=8;            //允许最大输出电压 即三个通道每个通道允许最大电压为8V==> 12 * PWM = 8V  所以占空比最大为2/3
	velocity_limit=200;         //最大转速
	voltage_sensor_align=6;     //传感器最大输入电压
    torque_controller=Type_voltage;  // 控制模式: 电压控制 目前只支持电压控制法，没有电流检测环
	controller=Type_angle;           // 当前控制模式：角度模式  还有速度模式、力矩模式
    target=0;
    my_timer_init();
    Motor_init(MOTOR_A,MOTOR_B,MOTOR_C); //对调任意两相即可对调电机旋转方向
	Motor_initFOC();
    PID_init();
    while(1)
    {
        switch(controller)
        {
            case Type_angle: move(target/57.3);  // 角度模式   读取滑动条的值 换算角度  move( float )输入参数 => 期望角度/57.3 
                                break;
            case Type_velocity:move(target/20);  // 速度模式   同下
                                break;
            case Type_torque:move(target/20);   // 力矩模式  不一定非要/20  程序是根据slider最大值255 即target<=255,换算得出255/20为最大转速，具体可根据需要自定义
                                break;
            default:move(target/57.3);          // 默认角度控制换算
                    break;
        }
		loopFOC();
    }
}
void log_info(char * text)
{
    print_MPU6050_data();
    print_CW2015_data(); //未插入锂电池时，会显示电量100%且电池电压通常大于4.2V以上，这是因为充电管理芯片的输出脚连接着锂电池的正极，因此采集的充电管理芯片的输出
                        //预计充电时间需要插入电池，且不插入充电线才能正确显示，且运行时间越长越准确
    sprintf(text,"MPU6050-> X:%.2f Y:%.2f Z:%.2f\n",gyro.gyro_x, gyro.gyro_y, gyro.gyro_z);
    if(ui_TextArea1!=NULL)lv_textarea_add_text(ui_TextArea1,text); 
    sprintf(text,"电池-> 电压:%.2fV 电量:%.2f%%\n       预计续航:%d分钟\n",bat_voltage,bat_percentage,remain_minutes);
    if(ui_TextArea1!=NULL)lv_textarea_add_text(ui_TextArea1,text);
    vTaskDelay(pdMS_TO_TICKS(2000));
}
void app_main(void)
{
    nvs_flash_init();
    read_foc_info();
    i2c_init();                                                                                  //初始化触摸IIC接口0
    gpio_adc_init();                                                                             //初始USB输入检测ADC引脚、软件关机引脚      
    CW2015_init();                                                                               //电量计初始化
    MPU6050_init();                                                                              //MPU6050陀螺仪初始化
    //user_init_sdcard();                                                                        //挂载SD卡时，必须配合插入SD卡使用，否则会报错
    //xTaskCreatePinnedToCore(print_rtos_status, "print_rtos_status", 1024*3, NULL, 3, NULL, 1); //打印内存信息
   xTaskCreatePinnedToCore(guiTask, "guiTask", 1024 * 24, NULL, 5, NULL, 0);                     //屏幕LVGL任务
   xTaskCreatePinnedToCore(foc_loop, "foc_loop", 1024 * 3, NULL, 4, NULL, 1);                    //FOC任务，不要在多线程环境下与CW2015_task和MPU6050_task一起开启，否则会造成IIC堵塞;                                                                                           
                                                                                                //FOC任务时，建议不要使用外部DC供电，使用板载升压电源即可;使用步进电机驱动时，可使用DC供电
   vTaskDelay(pdMS_TO_TICKS(3000));                                                             //等待LVGL初始化完成 
   char text[100];
   log_info(text);
   initialise_wifi();                                                                           //WIFI智能配网
   initialize_sntp();                                                                           //初始化RTC，获取网络时间
   while(!test_sntp());
   while(1)//离线信息打印，仅用demo演示，在这里进行lvgl对象操作，可能会影响屏幕流畅性，因为guiTask在核心0上多线程运行,非必要可以屏蔽
   {
        static uint8_t counter = 0 ;
        log_info(text);
        test_sntp();
        vTaskDelay(pdMS_TO_TICKS(10000));    
        counter ++;
        if(counter>=12){lv_textarea_set_text(ui_TextArea1,"");counter = 0;}///每两分钟清屏一次
   }
}

