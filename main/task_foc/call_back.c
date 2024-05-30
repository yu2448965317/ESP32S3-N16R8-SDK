#include "lvgl_gui.h"
void ui_event_Arc1(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
    if(event_code == LV_EVENT_VALUE_CHANGED) {
     //   _ui_arc_set_text_value(ui_Label_Celsius, target, "", "°");
    }
}
void ui_event_angle_Control(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
     //   fan_speed_value(e);
     target = lv_slider_get_value(ta); 
}
void ui_event_bright_Control(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, lv_slider_get_value(ta)));  //背光亮度调节 这里 < 8191 >是最大亮度 13位分辨率
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));
}
void ui_event_foc_mode_Control(lv_event_t * e)
{
    uint8_t option;
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    if(event_code == LV_EVENT_VALUE_CHANGED) {
       option = lv_roller_get_selected(ta);
       MagneticSensor_Init();
       switch (option)
       {
        case 0 :controller=Type_angle;
                lv_label_set_text(ui_Label1, "Angle:");
                    break;
        case 1 :controller=Type_torque;
                lv_label_set_text(ui_Label1, "Torque:");
                break;
        case 2 :controller=Type_velocity;
                lv_label_set_text(ui_Label1, "Velocity:");
                    break;
        default:controller=Type_angle;
                lv_label_set_text(ui_Label1, "Angle:");
                    break;
       }
    }
}
