#include "lvgl_gui.h"
#include <nvs.h>
#include <esp_partition.h>
#include <esp_log.h>

lv_obj_t * spinbox;
static void lv_spinbox_increment_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_user_data(e);
    if(code == LV_EVENT_SHORT_CLICKED || code  == LV_EVENT_LONG_PRESSED_REPEAT) {
        lv_spinbox_increment(ta);
    }
}
static void lv_spinbox_decrement_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_user_data(e);
    if(code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT) {
        lv_spinbox_decrement(ta);
    }
}
static void set_foc_param_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    PID * temp  = lv_event_get_user_data(e);
    lv_obj_t * ta = lv_event_get_target(e);
    *(temp->value)=(float)lv_spinbox_get_value(ta)/(float)100;
    save_config( temp->key, lv_spinbox_get_value(ta) );
    ESP_LOGI("FOC","%s%s%.2f",temp->key,":", *(temp->value));
}
void create_btn(lv_obj_t * obj,const char * label_text)
{
    lv_coord_t h = lv_obj_get_height(obj);
    lv_obj_t * btn_up = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn_up, h, h);
    lv_obj_align_to(btn_up, obj, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_obj_set_style_bg_img_src(btn_up, LV_SYMBOL_PLUS, 0);
    lv_obj_add_event_cb(btn_up, lv_spinbox_increment_event_cb, LV_EVENT_ALL, obj);
    lv_obj_t *  btn_down = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn_down, h, h);
    lv_obj_align_to(btn_down, obj, LV_ALIGN_OUT_LEFT_MID, -5, 0);
    lv_obj_set_style_bg_img_src(btn_down, LV_SYMBOL_MINUS, 0);
    lv_obj_add_event_cb(btn_down, lv_spinbox_decrement_event_cb, LV_EVENT_ALL, obj);
    
    lv_obj_t * label = lv_label_create(lv_scr_act());
    lv_obj_set_width(label, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(label, LV_SIZE_CONTENT);    /// 1
    lv_obj_align_to(label, btn_down, LV_ALIGN_OUT_LEFT_MID, -15, 0);
    lv_obj_set_style_text_font(label, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);    
    lv_obj_set_style_text_color(label, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(label,label_text);
}
void create_spinbox(lv_obj_t * table,const char * text,lv_coord_t x , lv_coord_t y, PID * p)
{
    lv_obj_t * spinbox = lv_spinbox_create(table);
    lv_spinbox_set_range(spinbox,-99999, 99999);
    lv_spinbox_set_digit_format(spinbox, 5, 3);
    lv_spinbox_step_prev(spinbox);
    lv_obj_set_style_text_font(spinbox, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);    
    lv_obj_set_style_text_color(spinbox, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(spinbox, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_width(spinbox, 100);
    lv_obj_center(spinbox);
    lv_obj_align(spinbox,LV_ALIGN_CENTER,x,y);
    lv_spinbox_set_value(spinbox,(uint32_t)(*(p->value)*100));
    lv_obj_add_event_cb(spinbox,set_foc_param_event_cb,LV_EVENT_VALUE_CHANGED,p);
    create_btn(spinbox,text);
}
void foc_pid_spinbox_init(void)
{
    create_spinbox(ui_Screen1,"V_P : ",-240,-10,&p_vel_p);
    create_spinbox(ui_Screen1,"V_I : ",-240,40,&p_vel_i);
    create_spinbox(ui_Screen1,"A_P : ",-240,90,&p_ang_p);
    create_spinbox(ui_Screen1,"A_D : ",-240,140,&p_ang_d);
    create_spinbox(ui_Screen1,"O_V : ",-240,190,&out_vel);
}