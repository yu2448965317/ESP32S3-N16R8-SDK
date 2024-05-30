
/******************************************************************************/
void PID_init(void);
float PID_velocity(float error);
float PID_angle(float error);
extern float pid_vel_P, pid_ang_P;
extern float pid_vel_I, pid_ang_D;
extern float integral_vel_prev;
extern float error_vel_prev, error_ang_prev;
extern float output_vel_ramp;
extern float output_vel_prev;
typedef struct 
{
  char * key ;
  float * value;

}PID;
extern PID p_vel_p,p_vel_i,p_ang_p,p_ang_d,out_vel;
/******************************************************************************/



