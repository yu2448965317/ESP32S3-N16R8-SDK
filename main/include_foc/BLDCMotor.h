
typedef enum
{
    CW      = 1,  //clockwise
    CCW     = -1, // counter clockwise
    UNKNOWN = 0   //not yet known or invalid state
} Direction;

/******************************************************************************/
extern float target;
extern long sensor_direction;
extern float voltage_power_supply;
extern float voltage_limit;
extern float voltage_sensor_align;
extern int  pole_pairs;
extern unsigned long open_loop_timestamp;
extern float velocity_limit;
/******************************************************************************/
void Motor_init(const int pinA, const int pinB, const int pinC);
void Motor_initFOC(void);
void loopFOC(void);
void move(float new_target);
void setPhaseVoltage(float Uq, float Ud, float angle_el);
