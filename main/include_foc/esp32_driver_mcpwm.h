
#include "driver/mcpwm.h"
#include "soc/mcpwm_reg.h"
#include "soc/mcpwm_struct.h"

// empty motor slot
#define _EMPTY_SLOT -20
#define _TAKEN_SLOT -21

// ABI bus frequency - would be better to take it from somewhere
// but I did nto find a good exposed variable
#define _MCPWM_FREQ 160e6f

// preferred pwm resolution default
#define _PWM_RES_DEF 4096
// min resolution
#define _PWM_RES_MIN 3000
// max resolution
#define _PWM_RES_MAX 8000
// pwm frequency
#define _PWM_FREQUENCY 25000 // default

// structure containing motor slot configuration
// this library supports up to 4 motors
typedef struct {
  int pinA;
  mcpwm_dev_t* mcpwm_num;
  mcpwm_unit_t mcpwm_unit;
  mcpwm_operator_t mcpwm_operator;
  mcpwm_io_signals_t mcpwm_a;
  mcpwm_io_signals_t mcpwm_b;
  mcpwm_io_signals_t mcpwm_c;
}bldc_3pwm_motor_slots_t;


typedef struct  {
  long pwm_frequency;
  mcpwm_dev_t* mcpwm_dev;
  mcpwm_unit_t mcpwm_unit;
  mcpwm_operator_t mcpwm_operator1;
  mcpwm_operator_t mcpwm_operator2;
}ESP32MCPWMDriverParams;
void _configure3PWM(const int pinA, const int pinB, const int pinC);
 
void _writeDutyCycle3PWM(float dc_a,  float dc_b, float dc_c);
