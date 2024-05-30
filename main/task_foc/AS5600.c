#include "lvgl_gui.h"

#define  cpr  4096
uint16_t angle=0;
float full_rotation_offset;
long  angle_data_prev;
unsigned long velocity_calc_timestamp;
float angle_prev;
uint16_t I2C_getRawCount()
{
    uint8_t angle_high = 0;
    uint8_t angle_low = 0;
    uint16_t result = 0;
    i2c_cmd_handle_t cmd;
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, As5600_Addr << 1 | I2C_WRITE_MODE, ACK_CHECK_ENABLE);
    i2c_master_write_byte(cmd, RawAngle_Addr, ACK_CHECK_ENABLE);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
   // vTaskDelay(1 / portTICK_PERIOD_MS);
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, As5600_Addr << 1 | I2C_READ_MODE, ACK_CHECK_ENABLE);
    i2c_master_read_byte(cmd, &angle_high, ACK); //0x0c是高位
    i2c_master_read_byte(cmd, &angle_low, NACK);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    result=(uint16_t)(angle_high<<8|angle_low); //一共就11位 注意
    //angle=((int) result & 0b0000111111111111)*360.0/4096.0;
    angle=(result*360.0/4096.0)+0.3;
	if (angle>=360)angle = 0 ;
    //ESP_LOGI("AS5600","%.2f",angle);
	return result;
}

void MagneticSensor_Init(void)
{
	angle_data_prev = I2C_getRawCount();  
	full_rotation_offset = 0;
	velocity_calc_timestamp=0;
}

float getAngle(void)
{
	float d_angle;
	uint16_t angle_data = I2C_getRawCount();
	// tracking the number of rotations 
	// in order to expand angle range form [0,2PI] to basically infinity
	d_angle = angle_data - angle_data_prev;
	// if overflow happened track it as full rotation
	if(fabs(d_angle) > (0.8*cpr) ) full_rotation_offset += d_angle > 0 ? -_2PI : _2PI; 
	// save the current angle value for the next steps
	// in order to know if overflow happened
	angle_data_prev = angle_data;
	return  (full_rotation_offset + ( angle_data / (float)cpr) * _2PI) ;
}

float getVelocity(void)
{
	unsigned long now_us;
	float Ts, angle_c, vel;

	// calculate sample time
	now_us = micros();
	if(now_us<velocity_calc_timestamp)Ts = (float)(velocity_calc_timestamp - now_us)/9*1e-6;
	else
		Ts = (float)(0xFFFFFF - now_us + velocity_calc_timestamp)/9*1e-6;
	// quick fix for strange cases (micros overflow)
	if(Ts == 0 || Ts > 0.5) Ts = 1e-3;

	// current angle
	angle_c = getAngle();
	// velocity calculation
	vel = (angle_c - angle_prev)/Ts;

	// save variables for future pass
	angle_prev = angle_c;
	velocity_calc_timestamp = now_us;
	return vel;
}




