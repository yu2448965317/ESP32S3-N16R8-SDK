#include "lvgl_gui.h"
#define _constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
bldc_3pwm_motor_slots_t esp32_bldc_3pwm_motor_slots[4] =  {
  {_EMPTY_SLOT, &MCPWM0, MCPWM_UNIT_0, MCPWM_OPR_A, MCPWM0A, MCPWM1A, MCPWM2A}, // 1st motor will be MCPWM0 channel A
  {_EMPTY_SLOT, &MCPWM0, MCPWM_UNIT_0, MCPWM_OPR_B, MCPWM0B, MCPWM1B, MCPWM2B}, // 2nd motor will be MCPWM0 channel B
  {_EMPTY_SLOT, &MCPWM1, MCPWM_UNIT_1, MCPWM_OPR_A, MCPWM0A, MCPWM1A, MCPWM2A}, // 3rd motor will be MCPWM1 channel A
  {_EMPTY_SLOT, &MCPWM1, MCPWM_UNIT_1, MCPWM_OPR_B, MCPWM0B, MCPWM1B, MCPWM2B}  // 4th motor will be MCPWM1 channel B
  };

ESP32MCPWMDriverParams params;
void _configureTimerFrequency(long pwm_frequency, mcpwm_dev_t* mcpwm_num,  mcpwm_unit_t mcpwm_unit){

  mcpwm_config_t pwm_config;
  pwm_config.counter_mode = MCPWM_UP_DOWN_COUNTER; // Up-down counter (triangle wave)
  pwm_config.duty_mode = MCPWM_DUTY_MODE_0; // Active HIGH
  pwm_config.frequency  = 2*pwm_frequency; // set the desired freq - just a placeholder for now https://github.com/simplefoc/Arduino-FOC/issues/76
  mcpwm_init(mcpwm_unit, MCPWM_TIMER_0, &pwm_config);    //Configure PWM0A & PWM0B with above settings
  mcpwm_init(mcpwm_unit, MCPWM_TIMER_1, &pwm_config);    //Configure PWM1A & PWM1B with above settings
  mcpwm_init(mcpwm_unit, MCPWM_TIMER_2, &pwm_config);    //Configure PWM2A & PWM2B with above settings


  vTaskDelay(pdMS_TO_TICKS(100));

  mcpwm_stop(mcpwm_unit, MCPWM_TIMER_0);
  mcpwm_stop(mcpwm_unit, MCPWM_TIMER_1);
  mcpwm_stop(mcpwm_unit, MCPWM_TIMER_2);

  // manual configuration due to the lack of config flexibility in mcpwm_init()
  mcpwm_num->clk_cfg.clk_prescale = 0;
  // calculate prescaler and period
  // step 1: calculate the prescaler using the default pwm resolution
  // prescaler = bus_freq / (pwm_freq * default_pwm_res) - 1
  int prescaler = ceil((double)_MCPWM_FREQ / (double)_PWM_RES_DEF / 2.0f / (double)pwm_frequency) - 1;
  // constrain prescaler
  prescaler = _constrain(prescaler, 0, 128);
  // now calculate the real resolution timer period necessary (pwm resolution)
  // pwm_res = bus_freq / (pwm_freq * (prescaler + 1))
  int resolution_corrected = (double)_MCPWM_FREQ / 2.0f / (double)pwm_frequency / (double)(prescaler + 1);
  // if pwm resolution too low lower the prescaler
  if(resolution_corrected < _PWM_RES_MIN && prescaler > 0 )
    resolution_corrected = (double)_MCPWM_FREQ / 2.0f / (double)pwm_frequency / (double)(--prescaler + 1);
  resolution_corrected = _constrain(resolution_corrected, _PWM_RES_MIN, _PWM_RES_MAX);

  // set prescaler
  mcpwm_num->timer[0].timer_cfg0.timer_prescale = prescaler;
  mcpwm_num->timer[1].timer_cfg0.timer_prescale = prescaler;
  mcpwm_num->timer[2].timer_cfg0.timer_prescale = prescaler;
  vTaskDelay(pdMS_TO_TICKS(1));
  //set period
  mcpwm_num->timer[0].timer_cfg0.timer_period = resolution_corrected;
  mcpwm_num->timer[1].timer_cfg0.timer_period = resolution_corrected;
  mcpwm_num->timer[2].timer_cfg0.timer_period = resolution_corrected;
  vTaskDelay(pdMS_TO_TICKS(1));
  mcpwm_num->timer[0].timer_cfg0.timer_period_upmethod = 0;
  mcpwm_num->timer[1].timer_cfg0.timer_period_upmethod = 0;
  mcpwm_num->timer[2].timer_cfg0.timer_period_upmethod = 0;
  vTaskDelay(pdMS_TO_TICKS(1));
  // _delay(1);
  //restart the timers
  mcpwm_start(mcpwm_unit, MCPWM_TIMER_0);
  mcpwm_start(mcpwm_unit, MCPWM_TIMER_1);
  mcpwm_start(mcpwm_unit, MCPWM_TIMER_2);
  vTaskDelay(pdMS_TO_TICKS(1));

  mcpwm_sync_config_t sync_conf = {
    .sync_sig = MCPWM_SELECT_TIMER0_SYNC,
    .timer_val = 0,
    .count_direction = MCPWM_TIMER_DIRECTION_UP
  };
  mcpwm_sync_configure(mcpwm_unit, MCPWM_TIMER_0, &sync_conf);
  mcpwm_sync_configure(mcpwm_unit, MCPWM_TIMER_1, &sync_conf);
  mcpwm_sync_configure(mcpwm_unit, MCPWM_TIMER_2, &sync_conf);

  // Enable sync event for all timers to be the TEZ of timer0
  mcpwm_set_timer_sync_output(mcpwm_unit, MCPWM_TIMER_0, MCPWM_SWSYNC_SOURCE_TEZ);
}


void _configure3PWM(const int pinA, const int pinB, const int pinC) {
  bldc_3pwm_motor_slots_t m_slot = {};

  // determine which motor are we connecting
  // and set the appropriate configuration parameters
  int slot_num;
  for(slot_num = 0; slot_num < 4; slot_num++){
    if(esp32_bldc_3pwm_motor_slots[slot_num].pinA == _EMPTY_SLOT){ // put the new motor in the first empty slot
      esp32_bldc_3pwm_motor_slots[slot_num].pinA = pinA;
      m_slot = esp32_bldc_3pwm_motor_slots[slot_num];
      break;
    }
  }

  mcpwm_gpio_init(m_slot.mcpwm_unit, m_slot.mcpwm_a, pinA);
  mcpwm_gpio_init(m_slot.mcpwm_unit, m_slot.mcpwm_b, pinB);
  mcpwm_gpio_init(m_slot.mcpwm_unit, m_slot.mcpwm_c, pinC);

  // configure the timer
  _configureTimerFrequency(_PWM_FREQUENCY, m_slot.mcpwm_num,  m_slot.mcpwm_unit);                                        
  params.pwm_frequency = _PWM_FREQUENCY,
  params.mcpwm_dev = m_slot.mcpwm_num,
  params.mcpwm_unit = m_slot.mcpwm_unit,
  params.mcpwm_operator1 = m_slot.mcpwm_operator;
}


void _writeDutyCycle3PWM(float dc_a,  float dc_b, float dc_c)
{
  // se the PWM on the slot timers
  // transform duty cycle from [0,1] to [0,100]
  mcpwm_set_duty(params.mcpwm_unit, MCPWM_TIMER_0, params.mcpwm_operator1, dc_a*100.0);
  mcpwm_set_duty(params.mcpwm_unit, MCPWM_TIMER_1, params.mcpwm_operator1, dc_b*100.0);
  mcpwm_set_duty(params.mcpwm_unit, MCPWM_TIMER_2, params.mcpwm_operator1, dc_c*100.0);
}


