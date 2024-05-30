/*USB输入检测引脚
                       USB_CHECK 
                           |
                           ↓
USB_IN------->6.8K电阻---------->3.4K电阻--------------------GND
                           |
                           ------100nF电容--------------------GND
*/
#define  USB_CHECK    1
/*******************************************************************/

/*软件开关机引脚,默认输出高电平
软件关机指令:60ms低电平+30ms高电平+120ms低电平+30ms高电平
软件息屏指令:60ms低电平+30ms高电平+30ms低电平+30ms高电平
*/
#define  SOFRWARE_SHUTDOWN   0

/*******************************************************************/

/*IIC定义*/
#define I2C_NUM   0  //IIC通道
#define I2C_SCL   4  //IIC->SCL引脚
#define I2C_SDA   5  //IIC->SDA引脚

/*******************************************************************/

/*LCD 8080并口引脚定义*/
#define PIN_NUM_DATA0 46
#define PIN_NUM_DATA1 3
#define PIN_NUM_DATA2 8
#define PIN_NUM_DATA3 18
#define PIN_NUM_DATA4 17
#define PIN_NUM_DATA5 16
#define PIN_NUM_DATA6 15
#define PIN_NUM_DATA7 7
#define PIN_NUM_PCLK 9
#define PIN_NUM_CS -1
#define PIN_NUM_DC 10
#define PIN_NUM_RST -1

/*******************************************************************/

/*屏幕背光调节引脚*/
#define PIN_NUM_BK_LIGHT 6

/*******************************************************************/

/*TF卡 SDMMC引脚定义*/
#define SD_CLK   14
#define SD_CMD   13
#define SD_D0   21
#define SD_D1   47
#define SD_D2   11
#define SD_D3   12
#define SD_CD   -1       

/*******************************************************************/

/*I2S音频总线定义*/
#define SCK     41
#define WS      42
#define DIN     2
#define DOUT    40 

/*******************************************************************/


/*无刷电机、步进电机定义驱动引脚定义，步进电机驱动引脚含义请看PDF文档  步进电机和无刷电机不能同时接入主板使用！！*/
#define MOTOR_A    38 //无刷电机A相驱动引脚 步进电机驱动HALF/FULL模式选择引脚 
#define MOTOR_B    39//无刷电机B相驱动引脚  步进电机驱动COLOCK (Step)引脚 
#define MOTOR_C    45//无刷电机C相驱动引脚  步进电机驱动CW/CCW模式选择引脚 
#define MOTOR_D    48//步进电机驱动ENABLE使能引脚，不使用步进电机时，请关闭此引脚！

/*******************************************************************/





